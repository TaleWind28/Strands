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
#define DURATA_PAUSA  60//60 secondi
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
int game_on = 0,ready = 0,game_starting,client_attivi = 0,score_time = 0; 
char classifica[2048];
int server_fd,client_fd;
time_t start_time,end_time;
pthread_t jester,scorer,main_tid;


/*MAIN DEL PROGRAMMA*/
void gestore_segnale(int signum) {
    int retvalue;
    if (signum == SIGINT){
        if (pthread_self() != main_tid)return;
        pthread_cancel(jester);
        
        //ammazza tutti i thread dei client
        pthread_mutex_lock(&player_mutex);
        while(Players!=NULL){
            pthread_t handler = Player_Peek_Hanlder(Players);
            pthread_cancel(handler);
            Player_Pop(&Players);

        }
        pthread_mutex_unlock(&player_mutex);
        printf("alla fine rimase solo il server\n");
        //ammazza tutti i client
        pthread_mutex_lock(&client_mutex);
        while (Client_List!=NULL){
            int death_sentence = L_Pop(&Client_List);
            Send_Message(death_sentence,"Ci scusiamo per il disagio ma dobbiamo terminare le attività, Grazie per aver Giocato\n",MSG_CHIUSURA_CONNESSIONE);
            writef(retvalue,"ammazzato\n");
        }
        pthread_mutex_unlock(&client_mutex);
        
        if (score_time == 1)pthread_cancel(scorer);
        writef(retvalue,"ammazzati tutti\n");
        SYSC(retvalue,shutdown(server_fd,SHUT_RDWR),"nello shutdown"); 
        writef(retvalue,"ammazzati tutti\n");
        SYSC(retvalue,close(server_fd),"chiusura dovuta a SIGINT");
        writef(retvalue,"terminazione dovuta a SIGINT\n");

        exit(EXIT_SUCCESS);
    }
    if (signum == SIGALRM){
        write(1, "ricevuto segnale SIGALRM\n", 25);
        char* time_string;
        switch(game_on){
            case 0:
                start_time = end_time; 
                //istanzio un allarme in base al parametro che mi viene passato
                alarm(parametri_server.durata_partita);
                //creo la stringa temporale
                time_string = tempo(parametri_server.durata_partita);
                Player_List tempor = Players;
                //invio la stringa ai giocatori
                for (int i =0;i<Player_Size(Players);i++){
                    pthread_t handler = Player_Peek_Hanlder(tempor);
                    int fd = Player_Retrieve_Socket(Players,handler);
                    Send_Message(fd,matrice_di_gioco,MSG_MATRICE);
                    Send_Message(fd,time_string,MSG_TEMPO_PARTITA);
                    tempor = tempor->next;
                }
                game_on = 1;
                printf("game on\n");
                break;
            case 1:
                time(&end_time);
                start_time = end_time;
                //durata pausa
                free(matrice_di_gioco);
                game_starting = 0;
                ready = 0;
                game_on = 0;
                score_time = 1;
                //dico a tutti i thread di mandare i risultati allo scorer
                Player_List temp = Players;
                for(int i = 0;i<Player_Size(Players);i++){
                    pthread_t handler = Player_Peek_Hanlder(temp);
                    SYST(retvalue,pthread_kill(handler,SIGUSR1),"nell'avviso di mandare il punteggio allo scorer");
                    temp = temp->next;
                }
                SYST(retvalue,pthread_create(&scorer,NULL,scoring,NULL),"nella creazione dello scorer");
                time_string = tempo(DURATA_PAUSA);
                Player_List tempot = Players;
                for (int i =0;i<Player_Size(Players);i++){
                    pthread_t handler = Player_Peek_Hanlder(tempot);
                    int fd2 = Player_Retrieve_Socket(Players,handler);
                    Send_Message(fd2,time_string,MSG_TEMPO_PARTITA);
                    tempot = tempot->next;
                }
                printf("game off\n");
                break;
        }
    }
    if (signum == SIGUSR1){
        char result[13];
        writef(retvalue,"Partita Conclusa\n");
        writef(retvalue,"entro\n");
        pthread_mutex_lock(&player_mutex);
        char* username = Player_Retrieve_User(Players,pthread_self());
        int points = Player_Retrieve_Score(Players,pthread_self());
        pthread_mutex_unlock(&player_mutex);
        sprintf(result,"%s,%d",username,points);
        writef(retvalue,"score\n");
        pthread_mutex_lock(&scorer_mutex);
        WL_Push(&Scoring_List,result);
        pthread_mutex_unlock(&scorer_mutex);
        Player_Update_Score(Players,pthread_self(),-points);
        score_time = 0;
    }
    if (signum == SIGUSR2){
        //vai a prendere il client fd in base al gestore
        int comm_fd = Player_Retrieve_Socket(Players,pthread_self());
        Send_Message(comm_fd,classifica,MSG_PUNTI_FINALI);
        if (pthread_self() == jester){
            pthread_exit(NULL);
        }
    }
}

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
    
    SYST(retvalue,pthread_mutex_init(&player_mutex,NULL),"nell'inizializzazione della player mutex");
    SYST(retvalue,pthread_mutex_init(&client_mutex,NULL),"nell'inizializzazione della client mutex");
    SYST(retvalue,pthread_mutex_init(&scorer_mutex,NULL),"nell'inizializzazione della scorer mutex");
    
    /*CREO UN THREAD PER GESTIRE LA CREAZIONE DEL SERVER ED IL DISPATCHING DEI THREAD*/
    SYST(retvalue,pthread_create(&jester,NULL,Gestione_Server,NULL),"nella creazione del giullare");
    
    int offset = 0;
    //int i =0;
    /*SFRUTTO IL SERVER COME DEALER*/
    while(1){
        if (ready == 0){
            Generate_Round(&offset);
            writef(retvalue,matrice_di_gioco);
            printf("\n");
            Print_Matrix(matrice_di_gioco,NUM_ROWS,NUM_COLUMNS,'?');
            ready = 1;
            matrice_grafo = Build_Graph(matrice_di_gioco,4,4);
        }
        if (Player_Size(Players)>0 && game_starting == 0){
            time(&start_time);
            alarm(DURATA_PAUSA);
            game_starting = 1;
        }
        //break;
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
                printf("Opzione non riconosciuta o mancante di argomento\n");
                break;
            default: printf("argomento superfluo ignorato\n");
        }
    }
    if (seed_given == 1 && params->matrix_file != NULL){
        perror("inserire soltanto 1 argomento opzionale tra seed e matrici");
        exit(EXIT_FAILURE);
    }
    srand(params->seed);
    return;
}

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

void Load_Dictionary(Trie* Dictionary, char* path_to_dict){
    FILE* dict = fopen(path_to_dict,"r");
    char word[256];
    while(fscanf(dict,"%s",word)!=EOF){
        Caps_Lock(word);
        insert_Trie(Dizionario,word);
    }
    return;
}

int Check_Words(char* string,off_t* offset, int file_descriptor){
    //char* stringcpy = strcpy(stringcpy,string);
    off_t cnt;
    for(int i = 0;i<strlen(string);i++){
        if (string[i] == '\n'){
            char* token = strtok(string,"\n");
            insert_Trie(Dizionario,token);
            cnt=0;
        }
        cnt++;
    }
    if (cnt == 1)return 0;
    *offset -= cnt;
    lseek(file_descriptor,*offset,SEEK_SET);
    return 0;   
}

/*THREAD CHE GESTISCE UN CLIENT*/
void* Thread_Handler(void* args){
    /*DICHIARAZIONE VARIABILI*/
    int retvalue,points = 0;char type = '0';char* input;char* username;
    /*RECUPERO IL VALORE PASSATO AL THREAD NELLA PTHREAD CREATE*/
    int client_fd = *(int*) args;
    Word_List parole_indovinate = NULL;
    pthread_mutex_lock(&client_mutex);
    L_Push(&Client_List,client_fd);
    if(L_Size(Client_List)> MAX_NUM_CLIENTS){
        printf("ciao\n");
        Send_Message(client_fd,"Ci scusiamo per il disagio ma il server al momento è pieno\n",MSG_CHIUSURA_CONNESSIONE);
        L_Pop(&Client_List);
        pthread_mutex_unlock(&client_mutex);
        //close(client_fd);
        return NULL;
        //Send_Message(client_fd,"Chiusura",MSG_CHIUSURA_CONNESSIONE);
    }
    pthread_mutex_unlock(&client_mutex);
   
    Send_Message(client_fd,WELCOME_MESSAGE,MSG_OK);
    //accetto solo la registrazione dell'utente
    username = Receive_Message(client_fd,&type);
    
    //controllo che l'username sia valido
    int exists = Player_Find_Word(Players,username);
    
    while((type != MSG_REGISTRA_UTENTE && type != MSG_CHIUSURA_CONNESSIONE) || exists == 0 || strlen(username)>10){
        free(username);
        if(strlen(username)>10)Send_Message(client_fd,"Username troppo lungo\n",MSG_ERR);
        if (exists == 0)Send_Message(client_fd,"Username già presente\n",MSG_ERR);
        else Send_Message(client_fd,"Inserisci il comando registra utente\n",MSG_ERR);
        username = Receive_Message(client_fd,&type);
        exists = Player_Find_Word(Players,username);
    }
    if (type == MSG_CHIUSURA_CONNESSIONE){
        writef(retvalue,"chiusura player\n");
        
        pthread_mutex_lock(&client_mutex);
        L_Splice(&Client_List);
        pthread_mutex_unlock(&client_mutex);
        
        return NULL;
    }
    Send_Message(client_fd,"Registrazione avvenuta con successo\n",MSG_OK);
    Send_Message(client_fd,RULES,MSG_OK);
    char* time_string;
    if (game_on == 1){
        //char* matrix_to_send = Stringify_Matrix(matrice_di_gioco);
        Send_Message(client_fd,matrice_di_gioco,MSG_MATRICE);
        time_string = tempo(parametri_server.durata_partita);
        Send_Message(client_fd,time_string,MSG_TEMPO_PARTITA);
    }else{
        // time(&start_time);
        time_string = tempo(DURATA_PAUSA);
        Send_Message(client_fd,time_string,MSG_TEMPO_ATTESA);
    }
    free(time_string);
    /*REGISTRO L'UTENTE NELLA TABELLA DEI GIOCATORT*/
    //aspetto la mutex per evitare race condition
    pthread_mutex_lock(&player_mutex);
    //inserisco player
    Player_Push_Thread(&Players,username,client_fd);
    //rilascio la mutex
    pthread_mutex_unlock(&player_mutex);
    //inizio a giocare
    while(type != MSG_CHIUSURA_CONNESSIONE){
        while (game_on !=1 && parole_indovinate !=NULL){
            WL_Pop(&parole_indovinate);
            points = 0-Player_Retrieve_Score(Players,pthread_self());
            Player_Update_Score(Players,pthread_self(),points);
        }

        //prendo l'input dell'utente
        input = Receive_Message(client_fd,&type);
        //Gioco con l'utente
        Choose_Action(client_fd,type,input,&parole_indovinate,&points);
        //libero l'input per il prossimo ciclo
        free(input);
    }
    //aspetto la mutex per eitare race condition
    pthread_mutex_lock(&player_mutex);
    //writef(retvalue,username);
    //rimuovo il player
    Player_Splice(&Players,username);
    //stampa per debug
    //Print_WList(Players);
    //rilascio la mutex per evitare il deadlock
    pthread_mutex_unlock(&player_mutex); 

    //stampa di debug
    writef(retvalue,"fine player\n");
    
    /*chiusura del socket*/
    //SYSC(retvalue,close(client_fd),"chiusura client-fake");
    return NULL;
}

void Choose_Action(int comm_fd, char type,char* input,Word_List* already_guessed,int* points){
    char *time_string;
    char*input_cpy = malloc(strlen(input));
    char* username;
    char* mess = malloc(35);
    int punteggio;
    //int retvalue;
    switch(type){
        case MSG_MATRICE:
            if(game_on !=1){
                time_string = tempo(DURATA_PAUSA);
                Send_Message(comm_fd,time_string,MSG_TEMPO_ATTESA);
                return;
            }//invia il tempo di attesa
            printf("game_on:%d\n",game_on);
            //trasforma la matrice in una stringa
            //matrix = Stringify_Matrix(matrice_di_gioco);
            //invio la matrice sotto forma di stringa al client
            Send_Message(comm_fd,matrice_di_gioco,MSG_MATRICE);
            //free(matrix);
            time_string = tempo(parametri_server.durata_partita);
            Send_Message(comm_fd,time_string,MSG_TEMPO_PARTITA);
            return;

        case MSG_PAROLA:
            
            if (game_on != 1){Send_Message(comm_fd,"Non puoi sottomettere parole perchè la partita deve ancora cominciare\n",MSG_ERR);return;}
            //accetto solo parole lunghe più di 4 caratteri
            if (strlen(input)<4) {Send_Message(comm_fd,"Parola troppo corta\n",MSG_ERR);return;}
            //salvo l'input in una copia per non distruggere la stringa originale
            strcpy(input_cpy,input);

            //rimuovo il carattere speciale dalla stringa
            Replace_Special(input_cpy,'Q');

            // //controllo se la parola è componibile nella matrice
            // if (Validate(matrice_di_gioco,input_cpy)!=0){Send_Message(comm_fd,"Parola Illegale su questa matrice\n",MSG_ERR);return;}
            if (dfs(matrice_grafo,input_cpy)!=true){Send_Message(comm_fd,"Parola Illegale su questa matrice\n",MSG_ERR);return;}
            //controllo parole già indovinate/*questo costa meno che cercare nel dizionario,però vva fatto in parallelo col pignoler*/
            if(WL_Find_Word(*already_guessed,input)==0){Send_Message(comm_fd,"Parola già inserita, 0 punti\n",MSG_PUNTI_PAROLA);return;}
            
            //controllo lessicale
            if(search_Trie(input,Dizionario)==-1){Send_Message(comm_fd,"la parola non esiste in italiano\n",MSG_ERR);return;}
            
            //inserisco la parola indovinata nella lista
            WL_Push(already_guessed,input);
            //comunico al client che la parola era corretta insieme al punteggio
            char message[buff_size];
            //aumento i punti
            Player_Update_Score(Players,pthread_self(),strlen(input));
            //calcolo i punti in base alla lunghezza della stringa
            sprintf(message,"Complimenti la parola che hai inserito vale %ld punti\n",strlen(input));
            //aggiorno il punteggio dell'utente
            //invio all'utente il messaggio con i suoi punti
            Send_Message(comm_fd,message,MSG_PUNTI_PAROLA);
            free(input_cpy);
            return;

        case MSG_REGISTRA_UTENTE:
            //dico all'utente che non serve registrarsi ulteriormente
            Send_Message(comm_fd,"Utente già Registrato\n",MSG_ERR);
            return; 

        case MSG_CHIUSURA_CONNESSIONE:
            //Send_Message(comm_fd,"ok",MSG_OK);
            username = Player_Retrieve_User(Players,pthread_self());
            //fare lock
            pthread_mutex_lock(&player_mutex);
            Player_Splice(&Players,username);
            pthread_mutex_unlock(&player_mutex);
            L_Splice(&Client_List);
            //togliere lock;
            printf("ammazzato\n");
            //SYSC(retvalue,listen(server_fd,1),"nell'aggiunta di una connessione");
            //non mando niente al client perchè potrebbe non essere più aperto il file descriptor di comunicazione
            return;
        
        case MSG_PUNTEGGIO:
            
            punteggio = Player_Retrieve_Score(Players,pthread_self());
            sprintf(mess,"il tuo punteggio attuale è %d\n",punteggio);
            Send_Message(comm_fd,mess,MSG_PUNTEGGIO);
            free(mess);
            return ;
        case MSG_PUNTI_FINALI:
            Send_Message(comm_fd,classifica,MSG_PUNTI_FINALI);
    }
}

void Replace_Special(char* string,char special){
    int len = strlen(string);
    int j = 0;
    for(int i =0;i<len;i++){
        if (string[i] != special){
            string[j++] = string[i];
        }else{
            string[j++] = '?';
            printf("carattere:%c\n",string[j]);
            i++;
        }
    }
    string[j] = '\0';
    printf("carattere:%s\n",string);
    return;
}

/*THREAD CHE GESTISCE LA CREAZIONE DEL SERVER E L'ACCETTAZIONE DEI GIOCATORI*/
void* Gestione_Server(void* args){
    /*dichiarazione variabili*/
    int retvalue;
    pthread_t client_thread[MAX_NUM_CLIENTS];
    //pthread_t acceptance_thread;
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
            //client_attivi++;
            /*DISPATCHING DI UN THREAD PER GESTIRE LA TRANSAZIONE*/
            SYST(retvalue,pthread_create(&client_thread[i],NULL,Thread_Handler,&client_fd),"dispatching dei thread");
            //SYST(retvalue,pthread_detach(client_thread[i]),"nella detach");
            i++;
        }
    return NULL;
}

void Init_SIGMASK(){
    int retvalue;
    struct sigaction azione_SIGINT;
    sigset_t maschera_segnale;
    sigemptyset(&maschera_segnale);
    //maschera seganle per SIGINT
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

int compare(const void *a, const void *b) {
    giocatore *playerA = (giocatore *)a;
    giocatore *playerB = (giocatore *)b;
    if(playerB->points<playerA->points)return -1;
    else return 1;
}

void* scoring(void* args){
    int cnt = 0;
    //int score[MAX_NUM_CLIENTS];
    int size = Player_Size(Players);
    giocatore global_score[size];
    //printf("hey\n");
    memset(classifica,0,strlen(classifica));
    while (1){
        //controllo di aver ricevuto tutti i dati
        if (cnt == size)break;
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
    char msg[1024];
    for(int i =0;i<cnt;i++){
        sprintf(msg,"user:%s\tpunteggio:%d\t\n",global_score[i].username,global_score[i].points);
        strcat(classifica,msg);
        //printf("%s\n",classifica);
    }
    int retvalue;
    pthread_mutex_lock(&player_mutex);
    Player_List temp = Players;
    pthread_mutex_unlock(&player_mutex);
    for(int i = 0;i<Player_Size(Players);i++){
        pthread_t handler = Player_Peek_Hanlder(temp);
        temp = temp->next;
        SYST(retvalue,pthread_kill(handler,SIGUSR2),"nell'avviso di mandare il punteggio allo scorer");
    }
    return NULL;
}

char* tempo(int max_dur){
    time(&end_time);
    double elapsed = difftime(end_time,start_time);
    double remaining = max_dur-elapsed; //+ 3;  
    char* mess = malloc(256);
    sprintf(mess,"%.0f secondi\n",remaining);
    return mess;
}