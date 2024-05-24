//questa serve per non avere gli error squiggles sulla sigaction
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
#include <signal.h>
#include <errno.h>
#include "../Header/macro.h"
#include "../Header/Matrix.h"
#include "../Header/Queue.h"
#include "../Header/Communication.h"

#define NUM_ROWS 4
#define NUM_COLUMNS 4
#define HELP_MESSAGE "Per prima cosa se non lo hai già fatto resgistrati mediante il comando registra_utente seguito dal tuo nome, poi potrai usare i seguenti comandi:\np seguito da una parola per indovinare una parola presente nella matrice che vedi a schermo\n matrice per visualizzare a schermo la matrice ed il tempo residuo di gioco\naiuto che ti mostra i comandi a te disponibili\nfine che ti fa uscire dalla partita in corso\n"
void Play(int client_fd);

int client_fd;

Matrix matrice_player;
void gestione_terminazione_errata(int signum) {
    char type;int retvalue;
    //invio un messaggio al server dicendogli che ho avuto un problema
    Send_Message(client_fd,"Ricevuto_Sigint",MSG_CHIUSURA_CONNESSIONE);
    //aspetto che mi risponda per evitare di chiiudere troppo presto il file descriptor
    Receive_Message(client_fd,&type);
    printf("sigint\n");
    /*chiudo il socket*/
    SYSC(retvalue,close(client_fd),"chiusura del cliente");
    exit(EXIT_SUCCESS);
}
int take_action(char* input, int comm_fd);

int main(int argc, char* argv[]){
    struct sigaction azione_segnale;
    sigset_t maschera_segnale;
    /*dichiarazione ed inizializzazione variabili*/
    int retvalue;
    struct sockaddr_in server_address;
    socklen_t server_len = sizeof(server_address);
    /*INIZIALIZZAZIONE VARIABILI IN BASE ALL'ARGOMENTO DELLA RIGA DI COMANDO*/
    char* HOST = argv[1];
    int PORT = atoi(argv[2]);

    /*CONTROLLO PARAMETRI RIGA DI COMANDO*/
    if(argc != 3){
        perror("usare la seguente sintassi: nome programa host porta_server");
        exit(EXIT_FAILURE);
    }
    /*inizializzazione server_address*/
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(HOST);
    server_address.sin_port = htons(PORT);
    
    /*creo il socket per comunicare*/
    SYSC(client_fd,socket(AF_INET,SOCK_STREAM,0),"nella creazione del client");
        
    /*chiamo la connect*/
    SYSC(retvalue,connect(client_fd,(struct sockaddr*)&server_address,server_len),"nella connect");
    writef(retvalue,"connesso\n");
     /*IMPOSTO LA MASCHERA*/
    sigemptyset(&maschera_segnale);
    SYSC(retvalue,sigaddset(&maschera_segnale,SIGINT),"aggiunta SIGINT alla maschera");

    /*IMPOSTO LA SIGACTION*/
    azione_segnale.sa_handler = gestione_terminazione_errata;
    azione_segnale.sa_mask = maschera_segnale;
  
    /*IMPOSTO IL GESTORE*/
    sigaction(SIGINT,&azione_segnale,NULL);
   
    //char type = '0';
    char input_buffer[buff_size];
    ssize_t n_read;
    matrice_player = Create_Matrix(4,4);
    while(1){
        SYSC(n_read,read(STDIN_FILENO,input_buffer,buff_size),"nella lettura dell'input utente");
        char* input = (char*)malloc(n_read+1);
        strncpy(input,input_buffer,n_read);
        input[n_read] = '\0';
        if (take_action(input,client_fd)==-1)break;
        free(input);
    }
    /*chiudo il socket*/
    SYSC(retvalue,close(client_fd),"chiusura del cliente");
}

int take_action(char* input, int comm_fd){
    //dichiarazione ed inizializzazione variabili
    int retvalue;
    char type = MSG_ERR,*answer,*matrice;
    char* token = strtok(input, " ");

    switch(input[0]){
        case 'a':
            writef(retvalue,HELP_MESSAGE);
            break;
        case 'r': 
            token = strtok(NULL,"\n");
            if (token == NULL || token[0] == '\n'){
                writef(retvalue,"nome utente non valido\n");
                break;
            }
            //invio al server il messaggio con le credenziali per la registrazione
            Send_Message(comm_fd,token,MSG_REGISTRA_UTENTE);
            //aspetto la risposta del server
            answer = Receive_Message(comm_fd,&type);
            //stampo all'utente la risposta
            writef(retvalue,answer);
            //pulisco la risposta
            free(answer);
            //controllo se la registrazione è andata a buon fine
            if (type == MSG_OK){
                //aspetto la matrice dal server
                matrice = Receive_Message(comm_fd,&type);
                if (type == MSG_TEMPO_ATTESA){
                    writef(retvalue,matrice);
                    break;
                }else{
                    //riempio la matrice
                    Fill_Matrix(matrice_player,matrice);
                    //stampo la matrice al client
                    writef(retvalue,"Questa è la matrice su cui giocare\n");
                    Print_Matrix(matrice_player,'?','Q');
                    char* tempo = Receive_Message(comm_fd,&type);
                    writef(retvalue,tempo);
                    //pulisco la stringa dove ho ricevuto la matrice
                    free(matrice);
                }
            }
            break;
        case 'm':
            //invio al server la richiesta della matrice
            Send_Message(comm_fd,"matrice",MSG_MATRICE);
            //aspetto la risposta del server
            matrice = Receive_Message(comm_fd,&type);
            //controllo di aver ricevuto la matrice
            if (type != MSG_MATRICE){writef(retvalue,matrice);free(matrice);break;}
            //in caso affermativo la riempio
            char mess[buff_size];
            sprintf(mess,"matrice:%s\n",matrice);
            writef(retvalue,mess);
            Fill_Matrix(matrice_player,matrice);
            //e la stampo all'utente
            Print_Matrix(matrice_player,'?','Q');
            //pulisco la stringa dove ho ricevuto la matrice
            free(matrice);
            char* time_string = Receive_Message(comm_fd,&type);
            writef(retvalue,time_string);
            break;
        case 'p':
            //tokenizzo la stringa per ottenere la parla inserita dall'utente
            token = strtok(NULL,"\n");
            //controllo che il token contenga qualcosa 
            if (token == NULL || token[0] == '\n'){
                //se il token è vuoto la parola è automaticamente sbagliata, quindi la considero come parola lunga 3 minuscola con il \n
                Send_Message(comm_fd,"lol\n",MSG_PAROLA);
            }else{
                //rendo maiuscola la stringa
                Caps_Lock(token);
                //invio al server la parola
                Send_Message(comm_fd,token,MSG_PAROLA);
            }
            //aspetto la risposta dal server
            answer = Receive_Message(comm_fd,&type);
            //stampo all'utente la risposta
            writef(retvalue,answer);
            //pulisco la variabile dove ho ricevuto la risposta dal server
            free(answer);
            break;
        case 'f':
            //comunico al server che l'utente si sta disconnettendo
            Send_Message(comm_fd,"fine",MSG_CHIUSURA_CONNESSIONE);
            Receive_Message(comm_fd,&type);
            //ritorno -1 per segnalare che devo terminare il client
            return -1;
            break;
        default:
            //stampa di default
            writef(retvalue,"comando non disponibile, digitare aiuto per una lista dettagliata\n");
            break;
    }   
    return 0;
}