#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "../Header/macro.h"
#include "../Header/Matrix.h"
#include "../Header/Queue.h"
#include "../Header/Communication.h"

#define NUM_ROWS 4
#define NUM_COLUMNS 4

void Play(int client_fd);
char* rec_mex(int Communication_fd,char Message_Type);

int main(int argc, char* argv[]){
    /*dichiarazione ed inizializzazione variabili*/
    int retvalue, client_fd;
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
    
    /*GIOCO LA PARTITA*/
    Play(client_fd);

    /*chiudo il socket*/
    SYSC(retvalue,close(client_fd),"chiusura del cliente");
}

void Play(int client_fd){
    /*DICHIARAZIONE VARIABILI*/
    int retvalue,i = 0;ssize_t n_read;char matrice_stringa[buff_size],score[buff_size];char msg_type = '0';
    /*CREO LA MATRICE DA FAR UTILIZZARE AL CLIENT*/
    Matrix client_matrix = Create_Matrix(NUM_ROWS,NUM_COLUMNS);
    /*LEGGO LA MATRICE PASSATA DAL SERVER*/
    //SYSC(retvalue,read(client_fd,matrice_stringa,buff_size),"nella lettura della matrice");
    strcpy(matrice_stringa,Receive_Message(client_fd,msg_type));
    /*RIEMPIO LA MATRICE COI DATI DEL SERVER*/
    Fill_Matrix(client_matrix,matrice_stringa);
    /*COSTRUISCO LA MAPPA DEI CARATTERI DELLA MATRICE*/
    Build_Charmap(client_matrix);
    /*STAMPO LA MATRICE A SCHERMO*/
    Print_Matrix(client_matrix,'Q','U');
    /*IL CLIENT PUO INIZIARE A GIOCARE*/
    while (1){
        /*TERMINAZIONE DELLA PARTITA DA CAMBIARE QUANDO AVRò LA SPECIFICA*/
        if (i == 5)break;
        /*STAMPA A SCHERMO*/
        writef(retvalue,"Inserisci una parola\n");
        /*ALLOCO UN PUNTATORE A CARATTERI DOVE MEMORIZZARE GLI INPUT UTENTE*/
        char* input = (char*)malloc(buff_size*sizeof(char));
        //char*score = (char*)malloc(buff_size*sizeof(char));
        /*ASPETTO UN INPUT DALL'UTENTE*/
        SYSC(n_read,read(STDIN_FILENO,input,buff_size),"nella lettura dell'input del cliente");
        //writef(retvalue,input);
        /*CONTROLLO SE L'UTENTE HA PER SBAGLIO INSERITO UN \n COME PAROLA*/
        if(n_read == 1){
            strcpy(input,"lol\n");
            n_read = 3;
        }
        input = realloc(input,n_read*sizeof(char));
        input = strtok(input,"\n");
        Caps_Lock(input);
        /*SCRIVO SUL FILE DESCRIPTOR CONDIVISO COL SERVER IL MESSAGGIO DA MANDARE*/
        Send_Message(client_fd,input,MSG_PAROLA);
        /*ASPETTO CHE MI VENGA COMUNICATO IL PUNTEGGIO*/
        //char* score = Receive_Message(client_fd,msg_type);
        SYSC(n_read,read(client_fd,score,buff_size),"nella lettura del punteggio");
        //char* score = Receive_Message(client_fd,msg_type);
        /*SCRIVO A VIDEO IL PUNTEGGIO*/
        i++;
        //writef(retvalue,"leggo");
        SYSC(retvalue,write(STDOUT_FILENO,score,n_read),"nella comunicazione del punteggio a video");
        free(input);
        //free(score);
        
    }
    /*leggo il totale*/
    SYSC(n_read,read(client_fd,score,buff_size),"nella lettura del punteggio totale");
    /*comunico il punteggio totale*/
    SYSC(retvalue,write(STDOUT_FILENO,score,strlen(score)),"nella comunicazione del punteggio totale");
    //writef(retvalue,score);
    return;
}