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
#include "../Header/Queue.h"
#include "../Header/Communication.h"
#include "../Header/Trie.h"

#define MAX_NUM_CLIENTS 32
#define NUM_ROWS 4
#define NUM_COLUMNS 4
#define DIZIONARIO "../Text/Dizionario.txt"
#define MATRICI "../Text/Matrici.txt"

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

/*THREAD FUNCTIONS*/
void* Thread_Handler(void* );
void* Gestione_Server(void* args);

/*GENERAL FUNCTIONS*/
void Init_Params(int argc, char*argv[],Parametri* params);
void Choose_Action(int client_fd,char type,char* input,Word_List* already_guessed,int points);
int Generate_Round();
void Load_Dictionary(Trie* Dictionary, char* path_to_dict);
void Replace_Special(char* string,char special);
/*GLOBAL VARIABLES*/
Parametri parametri_server;
Word_List Players;
Matrix matrice_di_gioco;
Trie* Dizionario;
pthread_mutex_t player_mutex;

char* HOST;
int PORT;
int game_on = 0; 
int server_fd;
//
/*MAIN DEL PROGRAMMA*/
void gestore_segnale(int signum) {
  int retvalue;
  if (signum == 2){
    
    SYSC(retvalue,close(server_fd),"chiusura dovuta a SIGINT");
  }
  write(1,"\nterminazione dovuta a SIGINT\n",31);
  exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]){
    /*DICIARAZIONE VARIABILI*/
    int retvalue;
    pthread_t jester;
    struct sigaction azione_segnale;
    sigset_t maschera_segnale;
    sigemptyset(&maschera_segnale);
    SYSC(retvalue,sigaddset(&maschera_segnale,SIGINT),"aggiunta SIGINT alla maschera");
    /*IMPOSTO LA SIGACTION*/
    azione_segnale.sa_handler = gestore_segnale;
    azione_segnale.sa_mask = maschera_segnale;
    /*IMPOSTO IL GESTORE*/
    sigaction(SIGINT,&azione_segnale,NULL);
    /*INIZIALIZZO I PARAMETRI PASSATI DA RIGA DI COMANDO, COMPRESI QUELLI OPZIONALI*/
    Init_Params(argc,argv,&parametri_server);
    //inizializzo la lista di giocatori
    Players = NULL;
    //inizializzo il dizionario
    Dizionario = create_node();
    //carico il dizionario in memoria
    Load_Dictionary(Dizionario,parametri_server.file_dizionario);
    
    //Print_Trie(Dizionario,buffer,0);

    //debug
    // int l = search_Trie("CIAO",Dizionario);
    // char mess[buff_size];
    // sprintf(mess,"%d\n",l);
    // writef(retvalue,mess);
    
    /*INIZIALIZZO LA MATRICE DI GIOCO*/
    matrice_di_gioco = Create_Matrix(NUM_ROWS,NUM_COLUMNS);
    
    /*CREO UN THREAD PER GESTIRE LA CREAZIONE DEL SERVER ED IL DISPATCHING DEI THREAD*/
    SYST(retvalue,pthread_create(&jester,NULL,Gestione_Server,NULL),"nella creazione del giullare");
    int ctr_value =0;
    int offset = 0;
    //int i =0;
    /*SFRUTTO IL SERVER COME DEALER*/
    while(ctr_value !=-1){
        /*BISOGNA SCRIVERE GENERATE ROUND IN MODO CHE QUANDO ARRIVA SIGINT SI GESTISCA la terminazione E SI CHIUDA*/
        ctr_value = Generate_Round(&offset);
        //sleep(60);
        break;
    }

    /*ASPETTO LA TERMINAZIONE DEL THREAD*/
    SYST(retvalue,pthread_join(jester,NULL),"nell'attesa del jester");
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
        {0, 0, 0, 0}
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
    params->durata_partita = 10;
    params->seed = -1;
    /*SCORRO TUTTI I PARAMETRI OPZIONALI RICEVUTI IN INPUT*/
    while((opt = getopt_long(argc,argv,"",logn_opt,&index))!=-1){
        switch(opt){
            case OPT_MATRICI:
                params->matrix_file = optarg;
                break;
            case OPT_DURATA:
                params->durata_partita = atoi(optarg);
                break;
            case OPT_SEED:
                params->seed = atoi(optarg);
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
    if (parametri_server.seed !=-1 && parametri_server.matrix_file != NULL){
        perror("NON SI PUò AVVIARE IL SERVER CON SIA IL SEED CHE IL FILE MATRICI\n");
        exit(EXIT_FAILURE);
    }
    return;
}

int Generate_Round(int* offset){
    //controllo se l'utente mi ha passato il file contenente le matrici
    char random_string[matrice_di_gioco.size+1];
    if(parametri_server.matrix_file != NULL){
        //leggo dal file in sequenza memorizzando l'offset,e carico la stringa nella matrice
        Load_Matrix(matrice_di_gioco,parametri_server.matrix_file,'Q',offset);
    }else{
        //se non ho il file genero casualmente
        srand(parametri_server.seed);
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
        random_string[matrice_di_gioco.size+1] ='\0';

        //carico la stringa nella matrice
        Fill_Matrix(matrice_di_gioco,random_string);
    }
    //stampo sul server la matrice/*DEBUG*/
    Print_Matrix(matrice_di_gioco,'?','Q');
    //costruisco la mappatura dei caratteri presenti nella matrice
    Build_Charmap(matrice_di_gioco);
    return 0;
}

void Load_Dictionary(Trie* Dictionary, char* path_to_dict){
    int retvalue,dizionario_fd;ssize_t n_read;struct stat file_stat;
    //invoco la stat per capire le dimensioni del file
    SYSC(retvalue,stat(path_to_dict,&file_stat),"nella stat del dizionario");
    //alloco un buffer per fare una sola lettura
    char* buffer = (char*)malloc(file_stat.st_size+1);
    SYSC(dizionario_fd,open(path_to_dict,O_RDONLY),"nell'apertura del dizionario");
    //leggo dal dizionario
    SYSC(n_read,read(dizionario_fd,buffer,file_stat.st_size),"nella lettura dal dizionario");
    //controllo di aver letto qualcosa
    if (n_read == 0)return;
    //metto l'ultimo elemento del buffer come terminatore nullo
    buffer[n_read] = '\0';
    Caps_Lock(buffer);
    //tokenizzo sul \n
    char* token = strtok(buffer,"\n");
    while(token!=NULL){
        insert_Trie(Dictionary,token);
        token = strtok(NULL,"\n");
    }
    free(buffer);
    SYSC(retvalue,close(dizionario_fd),"nella chiusura del dizionario");
    
    return;
}
/*THREAD CHE GESTISCE UN CLIENT*/
void* Thread_Handler(void* args){
    /*DICHIARAZIONE VARIABILI*/
    int retvalue,points = 0;char type = '0';char* input;char* username;
    /*RECUPERO IL VALORE PASSATO AL THREAD NELLA PTHREAD CREATE*/
    int client_fd = *(int*) args;
    Word_List parole_indovinate = NULL;
    //accetto solo la registrazione dell'utente
    username = Receive_Message(client_fd,&type);
    //controllo che l'username sia valido
    int exists = WL_Find_Word(Players,username);
    while((type != MSG_REGISTRA_UTENTE && type != MSG_CHIUSURA_CONNESSIONE) || exists == 0 || strlen(username)>10){
        free(username);
        if(strlen(username)>10)Send_Message(client_fd,"Username troppo lungo\n",MSG_ERR);
        if (exists == 0)Send_Message(client_fd,"Username già presente\n",MSG_ERR);
        else Send_Message(client_fd,"Inserisci il comando registra utente\n",MSG_ERR);
        username = Receive_Message(client_fd,&type);
        exists = WL_Find_Word(Players,username);
    }
    if (type == MSG_CHIUSURA_CONNESSIONE){SYSC(retvalue,close(client_fd),"chiusura client-fake");writef(retvalue,"chiusura player\n");return NULL;}
    Send_Message(client_fd,"Registrazione avvenuta con successo\n",MSG_OK);
    char* matrix_to_send = Stringify_Matrix(matrice_di_gioco);
    Send_Message(client_fd,matrix_to_send,MSG_MATRICE);

    /*REGISTRO L'UTENTE NELLA TABELLA DEI GIOCATORT*/
    //aspetto la mutex per evitare race condition
    pthread_mutex_lock(&player_mutex);
    //inserisco player
    WL_Push(&Players,username);
    //stampa di debug
    Print_WList(Players);
    //rilascio la mutex
    pthread_mutex_unlock(&player_mutex);

    while(type != MSG_CHIUSURA_CONNESSIONE){
        //prendo l'input dell'utente
        input = Receive_Message(client_fd,&type);
        //Gioco con l'utente
        Choose_Action(client_fd,type,input,&parole_indovinate,points);
        //libero l'input per il prossimo ciclo
        free(input);
    }
    //aspetto la mutex per eitare race condition
    pthread_mutex_lock(&player_mutex);
    //writef(retvalue,username);
    //rimuovo il player
    WL_Splice(&Players,username);
    //stampa per debug
    Print_WList(Players);
    //rilascio la mutex per evitare il deadlock
    pthread_mutex_unlock(&player_mutex); 

    //stampa di debug
    writef(retvalue,"fine player\n");
    
    /*chiusura del socket*/
    SYSC(retvalue,close(client_fd),"chiusura client-fake");
    return NULL;
}

void Choose_Action(int comm_fd, char type,char* input,Word_List* already_guessed,int points){
    char* matrix;
    char*input_cpy = malloc(strlen(input));
    
    switch(type){
        case MSG_MATRICE:
            //trasforma la matrice in una stringa
            matrix = Stringify_Matrix(matrice_di_gioco);
            //invio la matrice sotto forma di stringa al client
            Send_Message(comm_fd,matrix,MSG_MATRICE);
            free(matrix);
            return;

        case MSG_PAROLA:
            //salvo l'input in una copia per non distruggere la stringa originale
            strcpy(input_cpy,input);
            //rimuovo il carattere speciale dalla stringa
            Adjust_String(input_cpy,'U');
            Replace_Special(input_cpy,'Q');
            //controllo se la parola è componibile nella matrice
            if (Validate(matrice_di_gioco,input_cpy)!=0){Send_Message(comm_fd,"Parola Illegale su questa matrice\n",MSG_ERR);return;}
            
            //controllo parole già indovinate/*questo costa meno che cercare nel dizionario,però vva fatto in parallelo col pignoler*/
            if(WL_Find_Word(*already_guessed,input)==0){Send_Message(comm_fd,"Parola già inserita, 0 punti\n",MSG_PUNTI_PAROLA);return;}
            
            //controllo lessicale
            if(search_Trie(input,Dizionario)==-1){Send_Message(comm_fd,"la parola non esiste in italiano\n",MSG_ERR);return;}
            
            //inserisco la parola indovinata nella lista
            WL_Push(already_guessed,input);
            //comunico al client che la parola era corretta insieme al punteggio
            char message[buff_size];
            //aumento i punti
            points+= strlen(input);
            //calcolo i punti in base alla lunghezza della stringa
            sprintf(message,"Complimenti la parola che hai inserito vale %ld punti\n",strlen(input));
            //invio all'utente il messaggio con i suoi punti
            Send_Message(comm_fd,message,MSG_PUNTI_PAROLA);
            free(input_cpy);
            return;

        case MSG_REGISTRA_UTENTE:
            //dico all'utente che non serve registrarsi ulteriormente
            Send_Message(comm_fd,"Utente già Registrato\n",MSG_OK);
            return; 

        case MSG_CHIUSURA_CONNESSIONE:
            Send_Message(comm_fd,"ok",MSG_OK);
            //non mando niente al client perchè potrebbe non essere più aperto il file descriptor di comunicazione
            return;
    }
}

void Replace_Special(char* string,char special){
    int len = strlen(string);
    for(int i =0;i<len;i++){
        if (string[i] == special){
            string[i] = '?';
        }
    }
    return;
}
/*THREAD CHE GESTISCE LA CREAZIONE DEL SERVER E L'ACCETTAZIONE DEI GIOCATORI*/
void* Gestione_Server(void* args){
    /*dichiarazione variabili*/
    int retvalue, client_fd[MAX_NUM_CLIENTS];
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
    
    //accettazione dei client
    for(int i=0;i<MAX_NUM_CLIENTS;i++){
        /*accettazione delle richieste*/
        SYSC(client_fd[i],accept(server_fd,(struct sockaddr*)&client_address,&client_length),"nella accept");
        /*DISPATCHING DI UN THREAD PER GESTIRE LA TRANSAZIONE*/
        SYST(retvalue,pthread_create(&client_thread[i],NULL,Thread_Handler,&client_fd[i]),"dispatching dei thread");
    }
    /*ATTESA DELLA FINE DEI DISPATCHER*/
    for(int i =0;i<MAX_NUM_CLIENTS;i++){
        SYST(retvalue,pthread_join(client_thread[i],NULL),"nella join dei dispatcher");
    }
    /*CHIUSURA DEL SOCKET*/
    SYSC(retvalue,close(server_fd),"chiusura server");
    return NULL;
}