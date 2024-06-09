#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <getopt.h>
#include <time.h>
#include <signal.h>

#include "../Header/macro.h"
#include "../Header/Matrix.h"
#include "../Header/Stack.h"
#include "../Header/Communication.h"
#include "../Header/Trie.h"

#define MAX_NUM_CLIENTS 32
#define NUM_ROWS 4
#define NUM_COLUMNS 4
#define DIZIONARIO "../Text/Dizionario.txt"
#define MATRICI "../Text/Matrici.txt"
#define DURATA_PAUSA 2 //60 secondi
/*usata per debugging*/
//#define DURATA_PARTITA 5//60 secondi
#include <stdio.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    char* matrix_file;
    int durata_partita;
    long seed;
    char* file_dizionario;
}Parametri;

enum {
    OPT_MATRICI = 1,
    OPT_DURATA,
    OPT_SEED,
    OPT_DIZ,
};

typedef struct {
    char* username;
    int points;
}giocatore;

/*THREAD FUNCTIONS*/
void* Thread_Handler(void* args);
void* Gestione_Server(void* args);
void* scoring(void* args);

/*GENERAL FUNCTIONS*/
void Init_Params(int argc, char*argv[],Parametri* params);
void Init_SIGMASK();
void Choose_Action(int client_fd,char type,char* input,Word_List* already_guessed,int* points);
void Generate_Round();
void Load_Dictionary(Trie* Dictionary, char* path_to_dict);
void Replace_Special(char* string,char special);
int Check_Words(char* string,off_t* offset, int file_descriptor);
char* tempo(int max_dur);

/*GLOBAL VARIABLES*/
Parametri parametri_server;
Word_List Scoring_List;
Player_List Players;
char* matrice_di_gioco;
Trie* Dizionario;
pthread_mutex_t player_mutex,scorer_mutex,client_mutex;

List Client_List;
Graph* matrice_grafo;

char* HOST;
int PORT;
int game_on = 0,ready = 0,game_starting,client_attivi = 0,score_time = 0,unwanted_termination = 0; 
char classifica[2048];
int server_fd,client_fd;
time_t start_time;
pthread_t jester,scorer,main_tid;


//ISTANZIO IL GESTORE DEI SEGNALI
void gestore_segnale(int signum) {
    int retvalue;
    //CONTROLLO IL SEGNALE CHE HO RICEVUTO
    if (signum == SIGINT){
        //FACCIO RISPONDERE AL SIGINT SOLO IL THREAD DEL MAIN
        if (pthread_self() != main_tid)return;
        //TERMINO IL THREAD CHE GESTISCE LE CONNESSIONI DEI CLIENT
        SYST(retvalue,pthread_cancel(jester),"nell'ammazzamento del jester");
        //IMPEDISCO AL SERVER DI ASCOLTARE I MESSAGGI DAI CLIENT
        SYSC(retvalue,shutdown(server_fd,SHUT_RD),"nello shutdown"); 
        //CONTROLLO LO STATO DEL SERVER
        if (game_on == 1 && score_time == 0){
        //SE LA PARTITA È IN CORSO LA TERMINO PER STILARE LA CLASSIFICA
            unwanted_termination = 1;    
            //GENERO UN SIGALARM
            alarm(1);
            //STAMPA DI DEBUG
            printf("mandato alarma\n");
            //CREO LO SCORER
            pthread_create(&scorer,NULL,scoring,NULL);
            //E LO ASPETTO
            pthread_join(scorer,NULL);
        }
        //ammazza tutti i thread dei client
        pthread_mutex_lock(&player_mutex);
        //CICLO SU TUTTA LA LISTA ELIMINANDOLA
        while(Players!=NULL){
            //RECUPERO IL GESTORE
            pthread_t handler = Player_Peek_Hanlder(Players);
            //SE LA TERMINAZIONE AVVIENE IN PAUSA CANCELLO SEMPLICEMENTE IL THREAD, SENNò ASPETTO CHE COMUNICHI AL CLIENT LA CLASSIFICA
            if (unwanted_termination == 1){ SYST(retvalue,pthread_join(handler,NULL),"nell'attesa dei vari handler");}
            else pthread_cancel(handler);
            //ELIMINO IL GIOCATORE
            Player_Pop(&Players);
        }
        //RESTITUISCO LA MUTEX
        pthread_mutex_unlock(&player_mutex);
        //SE LA TERMINAZIONE AVVIENE IN STATO DI PAUSA DEVO PULIRE LA LISTA DI CLIENT
        if (unwanted_termination == 0){
            //ACQUISISCO LA MUTEX
            pthread_mutex_lock(&client_mutex);
            //ELIMINO TUTTI I CLIENT
            while(Client_List!= NULL){
                //RECUPERO IL FILE DESCRIPTOR PER COMUNICARE AL CLIENT LA CHIUSURA
                int death_sentence = L_Pop(&Client_List);
                //COMUNICO AL CLIENT LA CHIUSURA
                Send_Message(death_sentence,"Ci scusiamo per il disagio ma abbiamo risscontrato dei problemi e dobbiamo chiudere la connessione\nGrazie per aver Giocato\n",MSG_CHIUSURA_CONNESSIONE);
            }
            //RESTITUISCO LA MUTEX
            pthread_mutex_unlock(&client_mutex);
        }
        //IN CASO ABBIA AVVIATO LO SCORER LO TERMINO
        if (score_time == 1 && unwanted_termination == 0)SYST(retvalue,pthread_cancel(scorer),"AMMAZZAVO LO SCORER");
        //writef(retvalue,"ammazzati tutti\n");
        printf("alla fine rimase solo il server\n");
        
        SYSC(retvalue,close(server_fd),"chiusura dovuta a SIGINT");
        writef(retvalue,"terminazione dovuta a SIGINT\n");
        exit(EXIT_SUCCESS);
    }
    //CONTROLLO GLI STATI DI GIOCO
    if (signum == SIGALRM){
        //STAMPA DI DEBUG
        write(1, "ricevuto segnale SIGALRM\n", 25);
        char* time_string;
        //IN BASE ALLO STATO FACCIO COSE DIVERSE
        //NON HO BISOGNO DI CONTROLLARE CHE THREAD RISPONDE AL SEGNALE PERCHÈ IL SIGALARM VIENE INVIATO SOO AL THREAD CHE ISTANZIA L'ALLARME CHE È IL MAIN
        switch(game_on){
            //STATO DI PAUSA
            case 0:
                //MEMORIZZO L'ISTANTE DI TEMPO
                time(&start_time);
                //ISTANZIO UN ALLARME IN BASE ALLA DURATA CHE HO INIZIALIZZATO
                alarm(parametri_server.durata_partita);
                //CREO LA STRINGA DA INVIARE AI GIOCATOORI
                time_string = tempo(parametri_server.durata_partita);
                pthread_mutex_lock(&player_mutex);
                Player_List tempor = Players;
                pthread_mutex_unlock(&player_mutex);
                //INVIO LA STRINGA AI GIOCATORI SCORRENDO LA LISTA TEMPORANEA COME PER IL SIGINT
                for (int i =0;i<Player_Size(Players);i++){
                    //PRENDO IL GESTORE
                    pthread_t handler = Player_Peek_Hanlder(tempor);
                    //PRENDO IL FILE DESCRIPTOR
                    int fd = Player_Retrieve_Socket(Players,handler);
                    //MANDO LA MATRICE
                    Send_Message(fd,matrice_di_gioco,MSG_MATRICE);
                    //MANDO IL TEMPO
                    Send_Message(fd,time_string,MSG_TEMPO_PARTITA);
                    //SCORRO LA LISTA
                    tempor = tempor->next;
                }
                //CAMBIO LO STATO E LO FACCIO DIVENTARE STATO DI GIOCO
                game_on = 1;
                //STAMPA DI DEBUG
                printf("game on\n");
                break;
            //STATO DI GIOCO
            case 1:
                //MEMORIZZO L'ISTANTE DI TEMPO
                time(&start_time);
                //LIBERO LA MATRICE
                free(matrice_di_gioco);
                //COMUNICO CHE BISOGNA GENERARE UN'ALTRO ROUND
                game_starting = 0;
                ready = 0;
                //CAMBIO DI STATO DA STATO DI GIOCO A STATO DI PAUSA, L'ALLARME VERRà ISTANZIATO NEL MAIN
                game_on = 0;
                //COMUNICO CHE BISOGNA CHIAMARE LO SCORER
                score_time = 1;
                //SEMPRE CON LO STESSO MECCANISMO INVIO AI THREAD UN SEGNALE PER DIRLGI DI MANDARE IL RISULTATO ALLO SCORER E COMUNICO AI CLIENT IL TEMPO DI ATTESA
                Player_List temp = Players;
                time_string = tempo(DURATA_PAUSA);
                for(int i = 0;i<Player_Size(Players);i++){
                    pthread_t handler = Player_Peek_Hanlder(temp);
                    //MANDO UN SIGUSR1 DEFINITO DI SEGUITO
                    SYST(retvalue,pthread_kill(handler,SIGUSR1),"nell'avviso di mandare il punteggio allo scorer");
                    int fd2 = Player_Retrieve_Socket(temp,handler);
                    if (unwanted_termination == 1) Send_Message(fd2,"Abbiamo incotrato dei problemi, provvederemo a communicare la classifica attuale\n",MSG_SIGINT);
                    else Send_Message(fd2,time_string,MSG_TEMPO_ATTESA);
                    temp = temp->next;
                }
                //CONTROLLO CHE NON SIA ARRIVATO UN SIGINT, ED IN CASO NON FACCIO NIENTE PERCHÈ HO GIà CREATO LO SCORER
                if(unwanted_termination == 1)break;
                //CREO LO SCORER
                SYST(retvalue,pthread_create(&scorer,NULL,scoring,NULL),"nella creazione dello scorer");
                SYST(retvalue,pthread_detach(scorer),"nello staccare lo scorere");
                //STAMPA DI DEBUG
                printf("game off\n");
                break;
        }
    }
    //ARRIVA UN SEGNALE PERSONALIZZATO, LO MANDA IL MAIN DOPO LA ALARM IN STATO DI PAUSA
    if (signum == SIGUSR1){
        //PREPARO  UNA STRINGA SU CUI SCRIVERE I DATI PER LO SCORER
        char result[13];
        //STAMPE DI DEBUG
        writef(retvalue,"Partita Conclusa\n");
        writef(retvalue,"entro\n");
        //PRENDO LA MUTEX
        pthread_mutex_lock(&player_mutex);
        //PRENDO L'USERNAME DALLA LISTA
        char* username = Player_Retrieve_User(Players,pthread_self());
        //PRENDO IL PUNTEGGIO
        int points = Player_Retrieve_Score(Players,pthread_self());
        //RESTITUISCO LA MUTEX
        pthread_mutex_unlock(&player_mutex);
        //PREPARO IL MESSAGGIO
        sprintf(result,"%s,%d",username,points);
        //STAMPA DI DEBUG
        writef(retvalue,"score\n");
        //ACQUISISCO LA MUTEX DELLO SCORER
        pthread_mutex_lock(&scorer_mutex);
        //INSERISCO NELLA LISTA DELLO SCORER LA STRINGA APPENA COMPOSTA
        WL_Push(&Scoring_List,result);
        //RESTITUISCO LA MUTEX
        pthread_mutex_unlock(&scorer_mutex);
        //RESETTO IL PUNTEGGIO
        pthread_mutex_lock(&player_mutex);
        Player_Update_Score(Players,pthread_self(),-points);
        pthread_mutex_unlock(&player_mutex);
    }
    //ARRIVA UN'ALTRO SEGNALE PERSONALIZZATO, LO MANDA LO SCORER
    if (signum == SIGUSR2){
        //DICO CHE È FINITO IL TEMPO DELLO SCORER
        score_time = 0;
        //RECUPERO IL FILE DESCRIPTOR ASSOCIATO PER COMUNICARE CON L'UTENTE ASSOCIATO AL THREAD
        int comm_fd = Player_Retrieve_Socket(Players,pthread_self());
        //INVIO AL MIO CLIENT IL VINCITORE
        char* answer = malloc(2048); //= classifica;
        strcpy(answer,classifica);
        answer = strtok(answer,":");
        answer = strtok(NULL,"\t");
        char mess[buff_size];
        sprintf(mess,"%s è il vincitore\n",answer);
        writef(retvalue,mess);
        writef(retvalue,classifica);
        //INVIO AL MIO CLIENT IL VINCITORE
        Send_Message(comm_fd,mess,MSG_OK);
        //INVIO AL MIO CLIENT LA CLASSIFICA
        Send_Message(comm_fd,classifica,MSG_PUNTI_FINALI);
        //SE IL GIOCO STA TERMINANDO A CASUA DI UN SIGINT
        if(unwanted_termination == 1){
           
            //rimuovo il mio client dalla lista
            //ACQUISISCO LA MUTEX
            pthread_mutex_lock(&client_mutex);
            //RIMUOVO IL MIO CLIENT DALLA LISTA
            L_Splice(&Client_List);
            //RESTITUISCO LA MUTEX
            pthread_mutex_unlock(&client_mutex);
            //STAMPA DI DEBUG
            writef(retvalue,"ammazato\n");
            //COMUNICO AL CLIENT DI CHIUDERE LA CONNESSIONE
            Send_Message(comm_fd,"Grazie per aver Giocato\n",MSG_CHIUSURA_CONNESSIONE);
            //TERMINO L'ESECUZIONE
            pthread_exit(NULL);
            }
    }
}

/*MAIN DEL PROGRAMMA*/
int main(int argc, char* argv[]){
    /*DICHIARAZIONE VARIABILI*/
    int retvalue;
    //inizializzo la maschera per i segnali
    Init_SIGMASK();
    /*INIZIALIZZO I PARAMETRI PASSATI DA RIGA DI COMANDO, COMPRESI QUELLI OPZIONALI*/
    Init_Params(argc,argv,&parametri_server);
    //inizializzo la lista di giocatori
    Players = NULL;
    //inizializzo il dizionario
    Dizionario = create_node();
    /*INIZIALIZZO LA MATRICE DI GIOCO*/
    matrice_di_gioco =(char*)malloc(16*sizeof(char)); 
    main_tid  = pthread_self();
    //carico il dizionario in memoria
    Load_Dictionary(Dizionario,parametri_server.file_dizionario);
    writef(retvalue,"server_online\n");
    //INIZIALIZZO LE MUTEX
    SYST(retvalue,pthread_mutex_init(&player_mutex,NULL),"nell'inizializzazione della player mutex");
    SYST(retvalue,pthread_mutex_init(&client_mutex,NULL),"nell'inizializzazione della client mutex");
    SYST(retvalue,pthread_mutex_init(&scorer_mutex,NULL),"nell'inizializzazione della scorer mutex");
    
    /*CREO UN THREAD PER GESTIRE LA CREAZIONE DEL SERVER ED IL DISPATCHING DEI THREAD*/
    SYST(retvalue,pthread_create(&jester,NULL,Gestione_Server,NULL),"nella creazione del giullare");
    
    int offset = 0;
    //int i =0;
    /*SFRUTTO IL SERVER COME DEALER*/
    while(1){
        //FINCHÈ IL GIOCO NON PARTE O STA PARTENDO MEMORIZZO IL TEMPO CORRENTE
        if (Player_Size(Players) == 0 && game_starting == 0){
            time(&start_time);
        }
        //CONTROLLO DI NON AVER GIà PREPARATO UN ROUND
        if (ready == 0){
            //GENERO UN ROUND
            Generate_Round(&offset);
            //STAMPA DI DEBUG
            writef(retvalue,matrice_di_gioco);
            printf("\n");
            Print_Matrix(matrice_di_gioco,NUM_ROWS,NUM_COLUMNS,'?');
            //COSTURISCO IL GRAFO SUL QUALE CERCARE LE PAROLE
            matrice_grafo = Build_Graph(matrice_di_gioco,4,4);
            //COMUNICO DI AVER PREPARATO IL RUND
            ready = 1;
        }
        //ASPETTO IL PRIMO GIOCATORE
        if (Player_Size(Players)>0 && game_starting == 0){
            //MEMORIZZO L'ISTANTE DI TEMPO
            time(&start_time);
            //ISTANZIO UN ALLARME
            alarm(DURATA_PAUSA);
            //COMUNICO CHE IL GIOCO STA PER COMINCIARE
            game_starting = 1;
        }
    }
    return 0;
}
//
/*FINE MAIN*/
//

void Init_Params(int argc, char*argv[],Parametri* params){
    int opt, index = 0;
    /*DEFINISCO UNA STRUCT CON I PARAMETRI OPZIONALI CHE IL PROGRAMMA PUò RICEVERE*/
    struct option logn_opt[] = {
        {"matrici", required_argument, 0, OPT_MATRICI},
        {"durata", required_argument, 0, OPT_DURATA},
        {"seed", required_argument, 0, OPT_SEED},
        {"diz", required_argument, 0, OPT_DIZ},
        {0,0,0,0}
    };

    /*CONTROLLO PARAMETRI RIGA DI COMANDO*/
    if(argc < 3){
        perror("usare la seguente sintassi: nome programa host porta_server file_matrici");
        exit(EXIT_FAILURE);
    }

    /*INIZIALIZZAZIONE VARIABILI IN BASE ALL'ARGOMENTO DELLA RIGA DI COMANDO*/
    HOST = argv[1];
    PORT = atoi(argv[2]);
    /*valori di defualt in caso non venissero passati*/
    params->file_dizionario = DIZIONARIO;
    params->matrix_file = NULL;
    params->durata_partita = 180;
    params->seed = 0;
    int seed_given;
    /*SCORRO TUTTI I PARAMETRI OPZIONALI RICEVUTI IN INPUT*/
    while((opt = getopt_long(argc,argv,"",logn_opt,&index))!=-1){
        switch(opt){
            case OPT_MATRICI:
                params->matrix_file = optarg;
                break;
            case OPT_DURATA:
                params->durata_partita = atoi(optarg)*60;
                break;
            case OPT_SEED:
                params->seed = atoi(optarg);
                seed_given = 1;
                break;
            case OPT_DIZ:
                params->file_dizionario = optarg;
                break;
            case '?':
                // getopt_long già stampa un messaggio di errore
                printf("Argomento superfluo ignorato\n");
                break;
        }
    }
    if (seed_given == 1 && params->matrix_file != NULL){
        perror("inserire soltanto 1 argomento opzionale tra seed e matrici");
        exit(EXIT_FAILURE);
    }
    srand(params->seed);
    return;
}

//GENERO UN ROUND
void Generate_Round(int* offset){
    int retvalue;
    //controllo se l'utente mi ha passato il file contenente le matrici
    char random_string[(NUM_ROWS*NUM_COLUMNS)+1];
    if(parametri_server.matrix_file != NULL){
        //leggo dal file in sequenza memorizzando l'offset,e carico la stringa nella matrice
        matrice_di_gioco = Load_Matrix(parametri_server.matrix_file,'Q',offset);
        writef(retvalue,matrice_di_gioco);
    }else{
        //se non ho il file genero casualmente
        for (int i =0;i<16;i++){
            //genero un carattere random,modulo 26 perchè è 90-65 
            random_string[i] = (char)((rand()%26)+65);
            //controllo se la stringa ha una Q
            if (random_string[i] =='Q'){
                //inserisco un carattere speciale
                random_string[i] = '?';
            }
        }
        //termino la stringa
        random_string[(NUM_ROWS*NUM_COLUMNS)+1] ='\0';
        //carico la stringa nella matrice
        strncpy(matrice_di_gioco,random_string,16);
    }
    return;
}

//CARICO IL DIZIONARIO IN MEMORIA
void Load_Dictionary(Trie* Dictionary, char* path_to_dict){
    //APRO IL FILE TRAMITE IL PATH
    FILE* dict = fopen(path_to_dict,"r");
    //CREO UNA VARIABILE PER MEMORIZZARE LE PAROLE
    char word[256];
    //LEGGO TUTTO IL FILE
    while(fscanf(dict,"%s",word)!=EOF){
        //STANDARDIZZO LE PAROLE DEL DIZIONARIO METTENDOLE IN UPPERCASE
        Caps_Lock(word);
        //INSERISCO LA PAROLA NEL TRIE
        insert_Trie(Dizionario,word);
    }
    return;
}

/*THREAD CHE GESTISCE UN CLIENT*/
void* Thread_Handler(void* args){
    /*DICHIARAZIONE VARIABILI*/
    int retvalue,points = 0, exists = 0;;char type = '0';char* input;char* username;
    /*RECUPERO IL FILE DESCRIPTOR PASSATO AL THREAD NELLA PTHREAD CREATE PER POTER COMUNICARE COL CLIENT*/
    int client_fd = *(int*) args;
    Word_List parole_indovinate = NULL;
    pthread_mutex_lock(&client_mutex);
    L_Push(&Client_List,client_fd);
    if(L_Size(Client_List)> MAX_NUM_CLIENTS){
        printf("ciao\n");
        Send_Message(client_fd,"Ci scusiamo per il disagio ma il server al momento è pieno\n",MSG_CHIUSURA_CONNESSIONE);
        L_Pop(&Client_List);
        pthread_mutex_unlock(&client_mutex);
        return NULL;
    }
    //RILASCIO LA MUTEX
    pthread_mutex_unlock(&client_mutex);
    
    //INVIO AL CLIENT UN MESSAGGIO DI BENVENUTO
    Send_Message(client_fd,WELCOME_MESSAGE,MSG_OK);
    
    //ASPETTO CHE IL CLIENT MI INVII UN MESSAGGIO DI REGISTRAZIONE
    while(type != MSG_REGISTRA_UTENTE  || exists == 0 || strlen(username)>10){
        //RICEVO IL DATO DEL CLIENT
        username = Receive_Message(client_fd,&type);
        //CONTROLLO CHE NON CI SIA GIà
        exists = Player_Find_Word(Players,username);
        //CONTROLLO CHE IL MESSAGGIO NON SIA DI CHIUSURA
        if (type == MSG_CHIUSURA_CONNESSIONE){
            //STAMPA DI DEBUG
            writef(retvalue,"chiusura player\n");
            //ACQUISISCO LA MUTEX
            pthread_mutex_lock(&client_mutex);
            //RIMUOVO IL CLIENT DALLA LISTA
            L_Splice(&Client_List);
            //RILASCIO LA MUTEX
            pthread_mutex_unlock(&client_mutex);
            //TERMINO IL THREAD
            return NULL;
        }
        //CONTROLLO DI AVER RICEVUTO UN MESSAGGIO DI REGISTRAZIONE
        if (type!= MSG_REGISTRA_UTENTE){Send_Message(client_fd,"Inserisci il comando registra utente\n",MSG_ERR);free(username);continue;}
        //CONTROLLO CHE L'USERNAME NON SIA TROPPO LUNGO
        if(strlen(username)>10){Send_Message(client_fd,"Username troppo lungo\n",MSG_ERR);free(username);continue;}
        //CONTROLLO CHE L'USERNAME NON SIA GIà PRESENTE
        else if (exists == 0){Send_Message(client_fd,"Username già presente\n",MSG_ERR);free(username);continue;}
    }
    //ACQUISISCO LA MUTEX PER REGISTRARE L'UTENTE
    pthread_mutex_lock(&player_mutex);
    //INSERISCO L'UTENTE NELLA LISTA
    Player_Push_Thread(&Players,username,client_fd);
    //RILASCIO LA MUTEX
    pthread_mutex_unlock(&player_mutex);
    //COMUNICO AL CLIENT CHE LA REGISTRAZIONE È ANDATA A BUON FINE
    Send_Message(client_fd,"Registrazione avvenuta con successo\n",MSG_OK);
    //COMUNICO LE REGOLE AL CLIENT
    Send_Message(client_fd,RULES,MSG_OK);
    char* time_string;
    //CONTROLLO LO STATO DEL GIOCO
    if (game_on == 1){
        //SE IL GIOCO È IN CORSO INVIO LA MATRICE INSIEME AL TEMPO
        Send_Message(client_fd,matrice_di_gioco,MSG_MATRICE);
        time_string = tempo(parametri_server.durata_partita);
        Send_Message(client_fd,time_string,MSG_TEMPO_PARTITA);
    }else{
        //ALTRIMENTI SIAMO IN PAUSA QUINDI INVIO SOLO IL TEMPO
        time_string = tempo(DURATA_PAUSA);
        Send_Message(client_fd,time_string,MSG_TEMPO_ATTESA);
    }
    free(time_string);
    
    //INIZIO A GIOCARE CON L'UTENTE
    while(type != MSG_CHIUSURA_CONNESSIONE){
        
        //CONTROLLO DI ESSERE IN PAUSA E CHE LA LISTA DI PAROLE NON SIA VUOTA PER SVUOTARE LA LISTA DI PAROLE
        while (game_on !=1 && parole_indovinate !=NULL){
            //RIMUOVO LE PAROLE INDOVINATE UNA PER VOLTA
            WL_Pop(&parole_indovinate);
        }

        //ATTENDO L'INPUT DALL'UTENTE
        input = Receive_Message(client_fd,&type);
        
        //RISPONDO ALL'INPUT
        Choose_Action(client_fd,type,input,&parole_indovinate,&points);
        
        //LIBERO L'INPUT PER IL PROSSIMO CICLO
        free(input);
    }
    //ACQUISISCO LA MUTEX PER ACCEDERE ALLA LISTA DI GIOCATORI
    pthread_mutex_lock(&player_mutex);
    //RIMUOVO IL GIOCATORE
    Player_Splice(&Players,username);
    //RILASCIO LA MUTEX
    pthread_mutex_unlock(&player_mutex); 

    //stampa di debug
    writef(retvalue,"fine player\n");

    return NULL;
}

//SCELGO L'AZIONE IN BASE ALL'INPUT UTENTE
void Choose_Action(int comm_fd, char type,char* input,Word_List* already_guessed,int* points){
    char *time_string,* username,mess[buff_size];
    int punteggio;
    //CONTROLLO IL TIPO DEL MESSAGGIO
    switch(type){
        //MI HANNO RICHIESTO LA MATRICE
        case MSG_MATRICE:
            //SE LA PARTITA È IN PAUSA NON POSSO MANDARLA
            if(game_on !=1){
                //MANDO SOLO IL TEMPO DI ATTESA
                time_string = tempo(DURATA_PAUSA);
                Send_Message(comm_fd,time_string,MSG_TEMPO_ATTESA);
                return;
            }
            printf("game_on:%d\n",game_on);
            //INVIO LA MATRICE AL CLIENT
            Send_Message(comm_fd,matrice_di_gioco,MSG_MATRICE);
            //E SUCCESSIVAMENTE INVIO IL TEMPO RIMANENTE
            time_string = tempo(parametri_server.durata_partita);
            Send_Message(comm_fd,time_string,MSG_TEMPO_PARTITA);
            return;
        //MI HANNO INVIATO UNA PAROLA
        case MSG_PAROLA:
            //SE LA PARTITA È IN CORSO LA ACCETTO
            if (game_on != 1){Send_Message(comm_fd,"Non puoi sottomettere parole perchè la partita deve ancora cominciare\n",MSG_ERR);return;}
            //VENGONO ACCETTATE SOLO PAROLE LUNGHE ALMENO 4 CARATTERI
            if (strlen(input)<4) {Send_Message(comm_fd,"Parola troppo corta\n",MSG_ERR);return;}
            //CONTROLLO CHE LA APROLA ESISTA IN ITALIANO, FACCIO PRIMA QUESTO CONTROLLO PERCHÈ COSTA AL MASSIMO LA LUNGHEZZA DELLA PAROLA -> È PIù VELOCE DELLA DFS AL CASO PESSIMO
            if(search_Trie(input,Dizionario)==-1){Send_Message(comm_fd,"la parola non esiste in italiano\n",MSG_ERR);return;}
            //RIMPIAZZO LA Qu DELLA STRINGA CON UN CARATTERE SPECIALE
            Replace_Special(input,'Q');
            //CONTROLLO SE LA PAROLA È GIà STATA INDOVINATA, LO FACCIO PRIMA DELLA DFS SEMPRE PER MINIMIZZARE LE RICERCHE AL CASO PESSIMO
            if(WL_Find_Word(*already_guessed,input)==0){Send_Message(comm_fd,"Parola già inserita, 0 punti\n",MSG_PUNTI_PAROLA);return;}
            //CONTROLLO SE LA PAROLA È COMPONIBILE NELLA MATRICE TRAMITE UNA DFS
            if (dfs(matrice_grafo,input)!=true){Send_Message(comm_fd,"Parola Illegale su questa matrice\n",MSG_ERR);return;}
            //INSERISCO LA PAROLA TRA QUELLE INDOVINATE
            WL_Push(already_guessed,input);
            //COMUNICO AL CLIENT CHE LA PAROLA ERA CORRETTA E GLI RESTITUISCO IL PUNTEGGIO
            char message[buff_size];
            //AGGIORNO IL PUNTEGGIO DEL GIOCATORE
            Player_Update_Score(Players,pthread_self(),strlen(input));
            //CALCOLO I PUNTI IN BASE ALLA LUNGHEZZA DELLA STRINGA, QUESTO VIENE FATTO ANCHE PRIMA MA NON VIENE SALVATO IN UNA VARIABILE
            sprintf(message,"Complimenti la parola che hai inserito vale %ld punti\n",strlen(input));
            //INVIO ALL'UTENTE UN MESSAGGIO COI PUNTI
            Send_Message(comm_fd,message,MSG_PUNTI_PAROLA);
            return;
        //VOGLIONO REGISTRARSI DI NUOVO
        case MSG_REGISTRA_UTENTE:
            //RECUPERO IL NOME UTENTE USATO PER LA REGISTRAZIONE
            sprintf(mess,"Caro utente sei già registrato col nome %s\n",Player_Retrieve_User(Players,pthread_self()));
            //COMUNICO GENTILMENTE ALL'UTENTE CHE SI È GIà REGISTRATO 
            Send_Message(comm_fd,mess,MSG_ERR);
            //free(mess);
            return; 
        //VOGLIONO USCIRE DAL GIOCO
        case MSG_CHIUSURA_CONNESSIONE:
            //RECUPERO L'USERNAME DALLA LISTA
            username = Player_Retrieve_User(Players,pthread_self());
            //ELIMINO L'UTENTE DALLA LISTA
            Player_Splice(&Players,username);
            //RESTITUISCO LA MUTEX
            pthread_mutex_unlock(&player_mutex);
            //ACQUISISCO LA MUTEX
            pthread_mutex_lock(&client_mutex);
            //RIMUOVO IL CLIENT DALLA LISTA
            L_Splice(&Client_List);
            //RESTITUISCO LA MUTEX
            pthread_mutex_unlock(&client_mutex);
            //STAMPA DI DEBUG
            printf("ammazzato\n");
            return;
        //MI HANNO CHIESTO IL PUNTEGGIO
        case MSG_PUNTEGGIO:
            //RECUPERO IL PUNTEGGIO
            punteggio = Player_Retrieve_Score(Players,pthread_self());
            //PREPARO IL MESSAGGIO COL PUNTEGGIO
            sprintf(mess,"il tuo punteggio attuale è %d\n",punteggio);
            //INVIO AL CLIENT IL PUNTEGGIO
            Send_Message(comm_fd,mess,MSG_PUNTEGGIO);
            //LIBERO IL MESSAGGIO
            //free(mess);
            return ;
        //MI HANNO CHIESTO LA CLASSIFICA
        case MSG_PUNTI_FINALI:
            //INVIO AL CLIENT LA CLASSIFICA
            Send_Message(comm_fd,classifica,MSG_PUNTI_FINALI);
    }
}
//RIMPIAZZO UN CARATTERE CON IL CARATTERE SPECIALE
void Replace_Special(char* string,char special){
    //MEMORIZZO LA LUNGHEZZA DELLA STRINGA
    int len = strlen(string);
    int j = 0;
    //CICLO SUI CARATTERI DELLA STRINGA
    for(int i =0;i<len;i++){
        //FINCHÈ NON RILEVO IL CARATTERE DA SOSTITUIRE MEMORIZZO NORMALMENTE
        if (string[i] != special){
            string[j++] = string[i];
        }else{
            //APPENA LO RILEVO INSERISCO UN CARATTERE SPECIALE
            string[j++] = '?';
            i++;
        }
    }
    //TERMINO LA STRINGA
    string[j] = '\0';
    //STAMPA DI DEBUG
    printf("parola:%s\n",string);
    return;
}

/*THREAD CHE GESTISCE LA CREAZIONE DEL SERVER E L'ACCETTAZIONE DEI GIOCATORI*/
void* Gestione_Server(void* args){
    /*dichiarazione variabili*/
    int retvalue;
    pthread_t client_thread[MAX_NUM_CLIENTS];
    struct sockaddr_in server_address, client_address;
    socklen_t client_length = sizeof(client_address);

    /*inizializzazione server address*/
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(HOST);
    server_address.sin_port = htons(PORT);

    /*creazione socket*/
    SYSC(server_fd,socket(AF_INET,SOCK_STREAM,0),"creazione socket del server");
    
    /*binding dell'indirizzo*/
    SYSC(retvalue,bind(server_fd,(struct sockaddr*)&server_address,sizeof(server_address)),"nella bind");
    
    /*predisposizione del listner*/
    SYSC(retvalue,listen(server_fd,MAX_NUM_CLIENTS),"nella listen");
    int i = 0;
    //accettazione dei client
    while(1){
        if (i == MAX_NUM_CLIENTS)i = 0; 
        /*accettazione delle richieste*/
        SYSC(client_fd,accept(server_fd,(struct sockaddr*)&client_address,&client_length),"nella accept");
        /*DISPATCHING DI UN THREAD PER GESTIRE LA TRANSAZIONE*/
        SYST(retvalue,pthread_create(&client_thread[i],NULL,Thread_Handler,&client_fd),"dispatching dei thread");
        i++;
    }
    return NULL;
}

void Init_SIGMASK(){
    int retvalue;
    struct sigaction azione_SIGINT;
    sigset_t maschera_segnale;
    sigemptyset(&maschera_segnale);
    //GGIUNGO IL SET PER SIGIINT SIGALARM SIGUSR1 E SIGUSR2
    SYSC(retvalue,sigaddset(&maschera_segnale,SIGINT),"aggiunta SIGINT alla maschera");
    SYSC(retvalue,sigaddset(&maschera_segnale,SIGALRM),"aggiunta di SIGALARM alla maschera");
    SYSC(retvalue,sigaddset(&maschera_segnale,SIGUSR1),"aggiunta di SIGUSR1 alla maschera");
    SYSC(retvalue,sigaddset(&maschera_segnale,SIGUSR2),"aggiunta di SIGUSR1 alla maschera");

    /*IMPOSTO LA SIGACTION*/
    azione_SIGINT.sa_handler = gestore_segnale;
    azione_SIGINT.sa_mask = maschera_segnale;
    azione_SIGINT.sa_flags = SA_RESTART;
    /*IMPOSTO IL GESTORE*/
    sigaction(SIGINT,&azione_SIGINT,NULL);
    sigaction(SIGALRM,&azione_SIGINT,NULL);
    sigaction(SIGUSR1,&azione_SIGINT,NULL);
    sigaction(SIGUSR2,&azione_SIGINT,NULL);
    return;
}
//FUNZIONE PER USARE IL QSORT
int compare(const void *a, const void *b) {
    //CREO 2 GIOCATORI DA CONFRONTARE
    giocatore *playerA = (giocatore *)a;
    giocatore *playerB = (giocatore *)b;
    //ORDINO IN ORDINE CRESCENTE
    if(playerB->points<playerA->points)return -1;
    else return 1;
}
//THREAD SCORER
void* scoring(void* args){
    //INIZIALIZZO UNA VARIABILE CONTATORE
    int cnt = 0;
    //MEMORIZZO LA SIZE DEI GIOCATORI
    pthread_mutex_lock(&player_mutex);
    int size = Player_Size(Players);
    pthread_mutex_unlock(&player_mutex);
    giocatore global_score[size];
    //SVUOTO LA CLASSIFICA
    memset(classifica,0,strlen(classifica));
    
    while (cnt != size){
        //aspetta che la coda abbia degli elementi da prendere
        if (WL_Size(Scoring_List)>0){
            //acquisisce mutex
            pthread_mutex_lock(&scorer_mutex);
            //rimuove il dato dalla pila
            char* data = WL_Pop(&Scoring_List);
            //restitusce mutex
            pthread_mutex_unlock(&scorer_mutex);
            //tokenizza la stringa per ottenere username e punteggio
            char* token = strtok(data,",");
            global_score[cnt].username = token;
            token = strtok(NULL,",");
            global_score[cnt].points = atoi(token);
            //incremento il contatore
            cnt++;
        }   
    }
    //ordina l'array in base al più forte
    qsort(global_score,size, sizeof(giocatore), compare);
    //AVENDO AL MASSIMO 32 GIOCATORI AVREMMO PER OGNI GIOCATORE 5(user)+10(username)+10(punteggio)+1(score) = 28 CARATTERI AL MASSIMO QUINDI IL NUMERO MAX È 28*32 = 896 MA NON MI FIDO DEI MIEI CONTI QUINDI ALLOCO 1024
    char msg[32];
    //CREO LA STRINGA DELLA CLASSIFICA
    for(int i =0;i<cnt;i++){
        sprintf(msg,"user:%s\tpunteggio:%d\n",global_score[i].username,global_score[i].points);
        //CONCATENO LA STRINGA ALLA CLASSIFICA
        strcat(classifica,msg);
    }
    printf("Classifica pronta\n");
    int retvalue;
    //ACQUISISCO LA MUTEX
    pthread_mutex_lock(&player_mutex);
    //CREO UNA COPIA DELLA LISTA
    Player_List temp = Players;
    //RESTITUISCO LA MUTEX
    pthread_mutex_unlock(&player_mutex);
    //COMUNICO A TUTTI I THREAD CHE LA CLASSIFICA È PRONTA
    for(int i = 0;i<size;i++){
        //RECUPERO L'HANDLER
        pthread_t handler = Player_Peek_Hanlder(temp);
        //SCORRO LA LISTA
        temp = temp->next;
        //INVIO IL SENGALE AL THREAD
        SYST(retvalue,pthread_kill(handler,SIGUSR2),"nell'avviso di mandare il punteggio allo scorer");
    }
    //TERMINO LO SCORER
    return NULL;
}

//FUNZIONE CHE CALCOLA IL TEMPO RESTANTE
char* tempo(int max_dur){
    time_t end_time;
    //MEMORIZZA IL TEMPO ATTUALE
    time(&end_time);
    //CALCOLA LA DIFFERENZA TRA GLI ISTANTI DI TEMPO
    double elapsed = difftime(end_time,start_time);
    //CALCOLA QUANTO TEMPO RIMANE
    double remaining = max_dur-elapsed; //+ 3;  
    char* mess = malloc(256);
    //PREPARA UN MESSAGGIO CON DENTRO I SECONDI RIMANENTI
    sprintf(mess,"%.0f secondi\n",remaining);
    //E LO RITORNA
    return mess;
}