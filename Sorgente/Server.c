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
#include "../Header/Communication.h"

#define MAX_NUM_CLIENTS 32
#define NUM_ROWS 4
#define NUM_COLUMNS 4
#define GUESSED_SIZE 100
#define DIZIONARIO "../File Testuali/Dizionario.txt"
//#define MATRICI "../File Testuali/Matrici.txt"

void* Thread_Handler(void* );
void* Dealer(void *);
void* Pignoler(void *);
void Play(int );
void Build_Dictionary(Hash_Entry* ,char* );

Matrix playing_matrix;
Hash_Entry Dictionary[TABLE_SIZE];

int main(int argc, char* argv[]){
    /*CONTROLLO PARAMETRI RIGA DI COMANDO*/
    if(argc != 4){
        perror("usare la seguente sintassi: nome programa host porta_server file_matrici");
        exit(EXIT_FAILURE);
    }
    /*INIZIALIZZAZIONE VARIABILI IN BASE ALL'ARGOMENTO DELLA RIGA DI COMANDO*/
    char* HOST = argv[1];
    int PORT = atoi(argv[2]);
    char* MATRICI = argv[3];

    /*dichiarazione variabili*/
    int retvalue, server_fd , client_fd[MAX_NUM_CLIENTS];
    pthread_t client_thread[MAX_NUM_CLIENTS];
    pthread_t dealer_thread;
    //pthread_t acceptance_thread;
    struct sockaddr_in server_address, client_address;
    socklen_t client_length = sizeof(client_address);

    /*inizializzazione server address*/
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(HOST);
    server_address.sin_port = htons(PORT);
    /*inizializzo la tabella hash dove memorizzare il dizionario*/
    init_table(Dictionary,TABLE_SIZE);
    /*CREAZIONE DIZIONARIO*/
    Build_Dictionary(Dictionary,DIZIONARIO);

    /*creazione socket*/
    SYSC(server_fd,socket(AF_INET,SOCK_STREAM,0),"creazione socket del server");
    
    /*binding dell'indirizzo*/
    SYSC(retvalue,bind(server_fd,(struct sockaddr*)&server_address,sizeof(server_address)),"nella bind");
    
    /*predisposizione del listner*/
    SYSC(retvalue,listen(server_fd,MAX_NUM_CLIENTS),"nella listen");
    /*CREO UN THREAD PER REGOLAMENTARE LE PARTITE*/
    SYST(retvalue,pthread_create(&dealer_thread,NULL,Dealer,MATRICI),"nella creazione dell'arbitro");

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
    /*ATTESA DEL THREAD DEALER*/
    SYST(retvalue,pthread_join(dealer_thread,NULL),"attesa dealer");
    /*CHIUSURA DEL SOCKET*/
    SYSC(retvalue,close(server_fd),"chiusura server");
    return 0;
}

void* Thread_Handler(void* args){
    int retvalue;
    int client_fd = *(int*) args;
    /*faccio giocare il client*/
    Play(client_fd);
    writef(retvalue,"fine player\n");
    /*chiusura del socket*/
    SYSC(retvalue,close(client_fd),"chiusura client-fake");
    return NULL;
}

void* Dealer(void * args){
    int retvalue;
    char* MATRICI = (char*)args;
    /*attesa pre partita*/
    playing_matrix = Create_Matrix(NUM_COLUMNS,NUM_ROWS);
    /*Inizializza la matrice in base ad una riga del file matrici*/
    Load_Matrix(playing_matrix,MATRICI,'U');
    /*MAPPATURA DEI CARATTERI DELLA MATRICE*/
    Build_Charmap(playing_matrix);
    //Print_Matrix(playing_matrix);
    /*appena finisce l'attesa lo comunica ai thread*/
    printf("attesa finita\n");
    /*durata della partita*//*mettere una alarm*/
    sleep(30);
    /*aspetta che finisca la partita*/
    writef(retvalue,"fine partita\n");
    /*loop*/
    return NULL;
}

void Play(int client_fd){
    int retvalue, point = 0,highscore = 0,exit_val = 0;
    /*creo una tabella hash per memorizzare le parole indovinate dall'utente*/
    Hash_Entry parole_indovinate[GUESSED_SIZE];
    /*inizializzo la tabella hash*/
    init_table(parole_indovinate,GUESSED_SIZE);
    
    pthread_t thread_pignoler;
    /*ALLOCO UNA STRINGA DOVE ANDRO A MEMORIZARE LA MATRICE*/
    char* string_matrix = (char*)malloc(playing_matrix.size*(sizeof(char)));
    /*RENDO LA MATRICE DI GIOCO UNA STRINGA*/
    Stringify_Matrix(playing_matrix,string_matrix);
    writef(retvalue,"client connesso\n");

    /*MANDO LA MATRICE AL CLIENT*/
    char msg_type = MSG_OK;
    Send_Message(client_fd,string_matrix,MSG_MATRICE);
    while(1){
        /*LETTURA DAL CLIENT*/
        char* score = (char*)malloc(buff_size*sizeof(char));
        char* input = Receive_Message(client_fd,msg_type);
        /*STAMPE DI DEBUG*/
        writef(retvalue,input);
        writef(retvalue,":input\n");
        char mess[buff_size];
        sprintf(mess,"h1:%d\n",hash(input,TABLE_SIZE));
        writef(retvalue,mess)
        /*LANCIO IL PIGNOLER*/
        SYST(retvalue,pthread_create(&thread_pignoler,NULL,Pignoler,(void*)input),"nella creazione del pignoler");
        Adjust_String(input,'U');
        /*CONTROLLO DELLA VALIDITà DELLA PAROLA*/
        if (Validate(playing_matrix,input)==0){
            /*ASPETTO IL PIGNOLER*/
            SYST(retvalue,pthread_join(thread_pignoler,(void**)&exit_val),"nell'attesa del pignoler con parola valida");
            if (search_string(parole_indovinate,input,GUESSED_SIZE)!= 0 && exit_val == 0){
                insert_string(parole_indovinate,input,GUESSED_SIZE);
                /*CALCOLO IL PUNTEGGIO DELLA PAROLA*/
                point = strlen(input);
                /*MESSAGGIO LATO SERVER*/
                writef(retvalue,"parola legale\n");
                /*FORMATTAZZIONE PER LA STRINGA DA PASSARE AL CLIENT*/
                sprintf(score,"%d punti,complimenti\n",point);
                
            }else{
                /*ASSEGNO 0 PUNTI PERCHÈ LA PAROLA È GIA STATA USATA*//*stampa di debug lato server*/
                writef(retvalue,"parola già inserita\n");
                /*FORMATTAZZIONE PER LA STRINGA DA PASSARE AL CLIENT*/
                sprintf(score,"0 punti,ci hai provato brighella\n");
            }
        }else{/*PAROLA NON VALIDA*/
            /*stampa lato server per debug*/
            writef(retvalue,"parola illegale\n");
            /*FORMATTAZZIONE PER LA STRINGA DA PASSARE AL CLIENT*/
            sprintf(score,"0 punti,sagace\n");
            /*ASPETTO IL PIGNOLER*/
            SYST(retvalue,pthread_join(thread_pignoler,(void**)&exit_val),"nell'attesa del pignoler con la parola non valida");
            /*ASSEGNO 0 PUNTI PERCHÈ LA PAROLA NON È VALIDA*/
            point = 0;
        }
        
        /*PASSO LA STRINGA AL CLIENT*/
        //printf("parola:%s, punti:%d\n",input,point);
        //SYSC(retvalue,write(client_fd,score,strlen(score)),"nella comunicazione del punteggio");
        Send_Message(client_fd,score,MSG_PUNTI_PAROLA);
        /*AGGIUNGO IL PUNTEGGIO ATTUALE AL TOTALE*/
        highscore+=point;
        free(score);
    }
    char score[buff_size];
    sprintf(score,"hai totalizzato %d punti in tutta la partita\n",highscore);
    /*sincronizzo client e server*/
    usleep(2);
    Send_Message(client_fd,score,MSG_PUNTI_FINALI);
    //SYSC(retvalue,write(client_fd,score,buff_size),"nella comunicazione del punteggio totale");
    return;
}

/*THREAD CHE SCORRE IL DIZIONARIO PER CONTROLLARE LA VALIDITà DELLA PAROLA*/
void* Pignoler(void* args){
    int retvalue;
    /*recupero gli argomenti passati al server*/
    char* input = (char*) args;
    /*LEGGO DALLA CODA CONTENENTE IL DIZIONARIO*//*COSTA TROPPO FARE UNA READ VOLTA PER VOLTA*/
    writef(retvalue,input);
            char mess[buff_size];
        sprintf(mess,"h1:%d, h2:%d\n",hash(input,TABLE_SIZE),search_string(Dictionary,"CASI",TABLE_SIZE));
        writef(retvalue,mess)
    if(search_string(Dictionary,input,TABLE_SIZE)!=-1){
        writef(retvalue,"esco con 0");
        pthread_exit((void*)0);
        return NULL;
    }
    /*SE LA RICERCA NON HA SUCCESSO TERMINO IL THREAD E OTTENGO IL VALORE DI RITORNO*/
    writef(retvalue,"esco con -1");
    pthread_exit((void*)-1);
    return NULL;
}

void Build_Dictionary(Hash_Entry* table,char* path_to_dictionary){
    ssize_t n_read;
    int dizionario_fd;
    int file_size =279894 ;
    char buffer[file_size];
    char* token;
    int retvalue;
    /*APRO IL DIZIONARIO IN SOLA LETTURA*/
    SYSC(dizionario_fd,open(path_to_dictionary,O_RDONLY),"nell'apertura del dizionario");
    /*LEGGO LE PRIME 256 PAROLE DEL DIZIONARIO*/
    SYSC(n_read,read(dizionario_fd,buffer,file_size),"nella lettura dal dizionario");
    while(n_read>0){
        // Null-termina il buffer per garantire che sia una stringa valida
        token = strtok(buffer,"\n");
        while(token != NULL){
            /*INSERISCO LA PRIMA PAROLA NELLA LISTA*/
            Caps_Lock(token);
            writef(retvalue,token);
            writef(retvalue,"\n");
            insert_string(table,token,TABLE_SIZE);
            /*TOKENIZZO PER CERCARE LA PROSSIMA*/
            token = strtok(NULL,"\n");
        }
        
        SYSC(n_read,read(dizionario_fd,buffer,buff_size),"nella lettura dal dizionario");

    }
    SYSC(retvalue,close(dizionario_fd),"nella chiusura del file descriptor");
    return;
}