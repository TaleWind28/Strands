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
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include "../Header/macro.h"
#include "../Header/Matrix.h"
#include "../Header/Queue.h"

#define MAX_CLIENTS 3
#define NUM_ROWS 4
#define NUM_COLUMNS 4
#define DIZIONARIO "../File Testuali/Dizionario.txt"
#define MATRICI "../File Testuali/Matrici.txt"

void* Thread_Handler(void* );
void* Dealer(void *);
void* Pignoler(void *);
void Play();
void Build_Dictionary(Word_List* ,char* );
Matrix playing_matrix;
Word_List Dictionary = NULL;

int main(int argc, char* argv[]){
    /*CONTROLLO PARAMETRI RIGA DI COMANDO*/
    if(argc != 3){
        perror("usare la seguente sintassi: nome programa host porta_server");
        exit(EXIT_FAILURE);
    }
    /*INIZIALIZZAZIONE VARIABILI IN BASE ALL'ARGOMENTO DELLA RIGA DI COMANDO*/
    char* HOST = argv[1];
    int PORT = atoi(argv[2]);

    /*dichiarazione variabili*/
    int retvalue, server_fd , client_fd[MAX_CLIENTS];
    pthread_t client_thread[MAX_CLIENTS];
    pthread_t dealer_thread;
    //pthread_t acceptance_thread;
    struct sockaddr_in server_address, client_address;
    socklen_t client_length = sizeof(client_address);

    /*inizializzazione server address*/
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(HOST);
    server_address.sin_port = htons(PORT);
    
    /*CREAZIONE DIZIONARIO*/
    Build_Dictionary(&Dictionary,DIZIONARIO);
    
    /*creazione socket*/
    SYSC(server_fd,socket(AF_INET,SOCK_STREAM,0),"creazione socket del server");
    
    /*binding dell'indirizzo*/
    SYSC(retvalue,bind(server_fd,(struct sockaddr*)&server_address,sizeof(server_address)),"nella bind");
    
    /*predisposizione del listner*/
    SYSC(retvalue,listen(server_fd,MAX_CLIENTS),"nella listen");
    /*CREO UN THREAD PER REGOLAMENTARE LE PARTITE*/
    SYST(retvalue,pthread_create(&dealer_thread,NULL,Dealer,NULL),"nella creazione dell'arbitro");

    for(int i=0;i<MAX_CLIENTS;i++){
        /*accettazione delle richieste*/
        SYSC(client_fd[i],accept(server_fd,(struct sockaddr*)&client_address,&client_length),"nella accept");
        /*DISPATCHING DI UN THREAD PER GESTIRE LA TRANSAZIONE*/
        SYST(retvalue,pthread_create(&client_thread[i],NULL,Thread_Handler,&client_fd[i]),"dispatching dei thread");
    }
    /*ATTESA DELLA FINE DEI DISPATCHER*/
    for(int i =0;i<MAX_CLIENTS;i++){
        SYST(retvalue,pthread_join(client_thread[i],NULL),"nella join dei dispatcher");
    }
    /*ATTESA DEL THREAD DEALER*/
    SYST(retvalue,pthread_join(dealer_thread,NULL),"attesa dealer");
    /*CHIUSURA DEL SOCKET*/
    SYSC(retvalue,close(server_fd),"chiusura server");
    return 0;
}

void* Thread_Handler(void* args){
    int retvalue;
    int client_fd = *(int*) args;
    Play(client_fd);
    writef(retvalue,"fine player\n");
    /*chiusura del socket*/
    SYSC(retvalue,close(client_fd),"chiusura client-fake");
    return NULL;
}

void* Dealer(void * args){
    int retvalue;
    /*attesa pre partita*/
    playing_matrix = Create_Matrix(NUM_COLUMNS,NUM_ROWS);
    /*Inizializza la matrice in base ad una riga del file matrici*/
    Load_Matrix(playing_matrix,MATRICI);
    /*MAPPATURA DEI CARATTERI DELLA MATRICE*/
    Build_Charmap(playing_matrix);
    //Print_Matrix(playing_matrix);
    /*appena finisce l'attesa lo comunica ai thread*/
    printf("attesa finita\n");
    /*durata della partita*/
    sleep(30);
    /*aspetta che finisca la partita*/
    writef(retvalue,"fine partita\n");
    /*loop*/
    return NULL;
}

void Play(int client_fd){
    int retvalue, point = 0,highscore = 0,exit_val = 0;
    ssize_t n_read;char score[buff_size];
    Word_List parole_indovinate = NULL;
    pthread_t thread_pignoler;

    char* string_matrix = (char*)malloc(playing_matrix.size*(sizeof(char)));
    //Print_Matrix(playing_matrix);
    Stringify_Matrix(playing_matrix,string_matrix);
    //writef(retvalue,string_matrix);
    writef(retvalue,"client connesso\n");
    /*MANDO LA MATRICE AL CLIENT*/
    SYSC(retvalue,write(client_fd,string_matrix,strlen(string_matrix)),"nella comunicazione al client della matrice");
    for(int i=0;i<5;i++){
        /*ALLOCO UNA MATRICE DI CARATTERI PER MEMORIZZARE TUTTI GLI INPUT DELL'UTENTE*//*CAMBIARE ASSOLUTAMENTE APPENA FUNZIONA TUTTO*/
        //char** input = (char**)malloc(10*sizeof(char*));
        char* input = (char*)malloc(1024*sizeof(char));
        /*LETTURA DAL CLIENT*/
        SYSC(n_read,read(client_fd,input,1024),"nella lettura dal client");
        input = realloc(input,strlen(input)*sizeof(char));
        /*TOLGO IL \n DALLA STRINGA RICEVUTA IN INPUT*/
        char*token = strtok(input,"\n");
        /*LANCIO IL PIGNOLER*/
        SYST(retvalue,pthread_create(&thread_pignoler,NULL,Pignoler,token),"nella creazione del pignoler");
        /*CONTROLLO DELLA VALIDITà DELLA PAROLA*/
        if (Validate(playing_matrix,token)==0){
            /*ASPETTO IL PIGNOLER*/
            SYST(retvalue,pthread_join(thread_pignoler,(void**)&exit_val),"nell'attesa del pignoler con parola valida");
            if (Find_Word(parole_indovinate,token)!= 0 && exit_val == 0){
                Push(&parole_indovinate,token);
                /*CALCOLO IL PUNTEGGIO DELLA PAROLA*/
                point = strlen(token);
                /*MESSAGGIO LATO SERVER*/
                writef(retvalue,"parola legale\n");
                /*FORMATTAZZIONE PER LA STRINGA DA PASSARE AL CLIENT*/
                sprintf(score,"%d punti,complimenti\n",point);
            }else{
                writef(retvalue,"parola già inserita\n");
                /*ASSEGNO 0 PUNTI PERCHÈ LA PAROLA È GIA STATA USATA*/
                point = 0;
                /*FORMATTAZZIONE PER LA STRINGA DA PASSARE AL CLIENT*/
                sprintf(score,"%d punti,parola già inserita o illegale\n",point);
            }
        }else{/*PAROLA NON VALIDA*/
            writef(retvalue,"parola illegale\n");
            /*ASSEGNO 0 PUNTI PERCHÈ LA PAROLA NON È VALIDA*/
            point = 0;
            /*FORMATTAZZIONE PER LA STRINGA DA PASSARE AL CLIENT*/
            sprintf(score,"%d punti,sagace\n",point);
            /*ASPETTO IL PIGNOLER*/
            SYST(retvalue,pthread_join(thread_pignoler,NULL),"nell'attesa del pignole con la parola invalida");
        }
        
        /*PASSO LA STRINGA AL CLIENT*/
        SYSC(retvalue,write(client_fd,score,strlen(score)),"nella comunicazione del punteggio");
        /*AGGIUNGO IL PUNTEGGIO ATTUALE AL TOTALE*/
        highscore+=point;
    }
    //writef(retvalue,"fine partita\n");
    sprintf(score,"hai totalizzato %d punti in tutta la partita\n",highscore);
    SYSC(retvalue,write(client_fd,score,strlen(score)),"nella comunicazione del punteggio totale");
    return;
}

/*THREAD CHE SCORRE IL DIZIONARIO PER CONTROLLARE LA VALIDITà DELLA PAROLA*/
void* Pignoler(void* args){
    //int retvalue;
    //int exit_val = 0;
    char* input = (char*) args;
    /*LEGGO DALLA CODA CONTENENTE IL DIZIONARIO*//*COSTA TROPPO FARE UNA READ VOLTA PER VOLTA*/
    //Print_WList(Dictionary);
    //writef(retvalue,input);
    if(Find_Word(Dictionary,input)==0){
        pthread_exit((void*)0);
        return NULL;
    }
    //writef(retvalue,"diobo\n");
    /*SE LA RICERCA NON HA SUCCESSO*/
    //exit_val = -1;
    /*TERMINO IL THREAD E OTTENGO IL VALORE DI RITORNO*/
    pthread_exit((void*)-1);
    return NULL;
}

void Build_Dictionary(Word_List* wl,char* path_to_dictionary){
    ssize_t n_read;
    int dizionario_fd;
    char buffer[buff_size];

    /*APRO IL DIZIONARIO IN SOLA LETTURA*/
    SYSC(dizionario_fd,open(path_to_dictionary,O_RDONLY),"nell'apertura del dizionario");
    /*LEGGO LE PRIME 256 PAROLE DEL DIZIONARIO*/
    SYSC(n_read,read(dizionario_fd,buffer,buff_size),"nella lettura dal dizionario")
    while(n_read!=0){
        char* token = strtok(buffer,"\n");
        while(token != NULL){
            /*INSERISCO LA PRIMA PAROLA NELLA LISTA*/
            Push(wl,token);
            /*TOKENIZZO PER CERCARE LA PROSSIMA*/
            token = strtok(NULL,"\n");
        }
        SYSC(n_read,read(dizionario_fd,buffer,buff_size),"nella lettura dal dizionario")
    }
    return;
}