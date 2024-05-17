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

#include "../Header/macro.h"
#include "../Header/Matrix.h"
#include "../Header/Queue.h"
#include "../Header/Communication.h"

#define MAX_NUM_CLIENTS 32
#define NUM_ROWS 4
#define NUM_COLUMNS 4
#define DIZIONARIO "../File Testuali/Dizionario.txt"
#define MATRICI "../File Testuali/Matrici.txt"

typedef struct {
    char* matrix_file;
    int durata_partita;
    long seed;
    char* file_dizionario;
}Parametri;

typedef struct Player{
    char* username;
    pthread_t gestore;
    int currently_playing;
}Player;

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
void Choose_Action(char type);
int Generate_Round();

/*GLOBAL VARIABLES*/
Parametri parametri_server;
Player giocatori[MAX_NUM_CLIENTS];
Hash_Entry Tabella_Player[MAX_NUM_CLIENTS];
char* HOST;
int PORT;

int main(int argc, char* argv[]){
    /*DICIARAZIONE VARIABILI*/
    int retvalue;
    pthread_t jester;
    
    /*INIZIALIZZO I PARAMETRI PASSATI DA RIGA DI COMANDO, COMPRESI QUELLI OPZIONALI*/
    Init_Params(argc,argv,&parametri_server);
    /*prova tabella hash*/
    init_table(Tabella_Player,MAX_NUM_CLIENTS);

    /*CREO UN THREAD PER GESTIRE LA CREAZIONE DEL SERVER ED IL DISPATCHING DEI THREAD*/
    SYST(retvalue,pthread_create(&jester,NULL,Gestione_Server,NULL),"nella creazione del giullare");
    
    /*SFRUTTO IL SERVER COME DEALER*/
    while(1){
        int ctr_value = Generate_Round();/*BISOGNA SCRIVERE GENERATE ROUND IN MODO CHE QUANDO ARRIVA SIGINT SI GESTISCA TUTTO E SI CHIUDA*/
        if (ctr_value == -1) break;
    }

    /*ASPETTO LA TERMINAZIONE DEL THREAD*/
    SYST(retvalue,pthread_join(jester,NULL),"nell'sattesa del jester");
    return 0;
}

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
    if(argc < 4){
        perror("usare la seguente sintassi: nome programa host porta_server file_matrici");
        exit(EXIT_FAILURE);
    }

    /*INIZIALIZZAZIONE VARIABILI IN BASE ALL'ARGOMENTO DELLA RIGA DI COMANDO*/
    HOST = argv[1];
    PORT = atoi(argv[2]);

    /*SCORRO TUTTI I PARAMETRI OPZIONALI RICEVUTI IN INPUT*/
    while((opt =getopt_long(argc,argv,"",logn_opt,&index))!=-1){
        switch(opt){
            case OPT_MATRICI:
                params->matrix_file = optarg;
                //printf("matrice:%s\n",optarg);
                break;
            case OPT_DURATA:
                params->durata_partita = atoi(optarg);
                //printf("durata:%s\n",optarg);
                break;
            case OPT_SEED:
                params->seed = atoi(optarg);
                //printf("seed:%s\n",optarg);
                break;
            case OPT_DIZ:
                params->file_dizionario = optarg;
                //printf("dizionario:%s\n",optarg);
                break;
            case '?':
                // getopt_long già stampa un messaggio di errore
                printf("Opzione non riconosciuta o mancante di argomento\n");
                break;
            default: printf("argomento superfluo ignorato\n");
        }
    }
    if (argc==4){
        params->matrix_file = MATRICI;
        params->durata_partita = 10;
        params->seed = 1;
        params->file_dizionario = DIZIONARIO;
    }
    return;
}

int Generate_Round(){
    sleep(2);return-1;

    return 0;
}

/*THREAD CHE GESTISCE UN CLIENT*/
void* Thread_Handler(void* args){
    int retvalue;
    int client_fd = *(int*) args;
    char type = '0';
    

     //accetto solo la registrazione dell'utente
    char* username = Receive_Message(client_fd,&type);
    while(type != MSG_REGISTRA_UTENTE){
        free(username);
        Send_Message(client_fd,"Inserisci il comando registra utente\n",MSG_ERR);
        username = Receive_Message(client_fd,&type);
    }
    Send_Message(client_fd,"Registrazione avvenuta con successo\n",MSG_OK);

    /*REGISTRO L'UTENTE NELLA TABELLA DEI GIOCATORT*/
    insert_string(Tabella_Player,username,MAX_NUM_CLIENTS);
    char* input;
    while(type != MSG_CHIUSURA_CONNESSIONE){
        //prendo l'input dell'utente
        input = Receive_Message(client_fd,&type);
        //Gioco con l'utente
        Choose_Action(type);
        //libero l'input per il prossimo ciclo
        free(input);
        break;
    }
    writef(retvalue,&type);
    writef(retvalue,"fine player\n");
    /*chiusura del socket*/
    SYSC(retvalue,close(client_fd),"chiusura client-fake");
    return NULL;
}

void Choose_Action(char type){
    return;
}

/*THREAD CHE GESTISCE LA CREAZIONE DEL SERVER E L'ACCETTAZIONE DEI GIOCATORI*/
void* Gestione_Server(void* args){
    /*dichiarazione variabili*/
    int retvalue, server_fd , client_fd[MAX_NUM_CLIENTS];
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
        giocatori[i].gestore = client_thread[i];
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