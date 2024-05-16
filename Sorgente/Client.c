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

int main(int argc, char* argv[]){
    /*dichiarazione ed inizializzazione variabili*/
    int retvalue, client_fd;char matrice_stringa[buff_size];char msg_type = '0';
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
    
    /*CREO LA MATRICE DA FAR UTILIZZARE AL CLIENT*/
    Matrix client_matrix = Create_Matrix(NUM_ROWS,NUM_COLUMNS);

    /*LEGGO LA MATRICE PASSATA DAL SERVER*/
    strcpy(matrice_stringa,Receive_Message(client_fd,msg_type));
    /*RIEMPIO LA MATRICE COI DATI DEL SERVER*/
    Fill_Matrix(client_matrix,matrice_stringa);
    /*COSTRUISCO LA MAPPA DEI CARATTERI DELLA MATRICE*/
    Build_Charmap(client_matrix);
    /*STAMPO LA MATRICE A SCHERMO*/
    Print_Matrix(client_matrix,'Q','u');
    /*GIOCO LA PARTITA*/
    Play(client_fd);

    /*chiudo il socket*/
    SYSC(retvalue,close(client_fd),"chiusura del cliente");
}

void Play(int client_fd){
    /*DICHIARAZIONE VARIABILI*/
    int retvalue;ssize_t n_read;char msg_type = '0';
    
    /*IL CLIENT PUO INIZIARE A GIOCARE*/
    while (1){
        /*TERMINAZIONE DELLA PARTITA DA CAMBIARE QUANDO AVRò LA SPECIFICA*/
        /*STAMPA A SCHERMO*/
        writef(retvalue,"Prompt Paroliere\n");
        /*ALLOCO UN PUNTATORE A CARATTERI DOVE MEMORIZZARE GLI INPUT UTENTE*/
        char* input = (char*)malloc(buff_size);
        /*ASPETTO UN INPUT DALL'UTENTE*/
        SYSC(n_read,read(STDIN_FILENO,input,buff_size),"nella lettura dell'input del cliente");
        /*CONTROLLO SE L'UTENTE HA PER SBAGLIO INSERITO UN \n COME PAROLA*/
        if(n_read == 1){
            strcpy(input,"l\n");
            n_read = 1;
        }
        // writef(retvalue,input);
        char* token = strtok(input,"\n");
        Caps_Lock(token);
        /*SCRIVO SUL FILE DESCRIPTOR CONDIVISO COL SERVER IL MESSAGGIO DA MANDARE*/
        Send_Message(client_fd,token,MSG_PAROLA);
        /*ASPETTO CHE MI VENGA COMUNICATO IL PUNTEGGIO*/
        char* score = Receive_Message(client_fd,msg_type);
        /*SCRIVO A VIDEO IL PUNTEGGIO*/
        SYSC(retvalue,write(STDOUT_FILENO,score,strlen(score)),"nella comunicazione del punteggio a video");
        free(input);
    }
    /*leggo il totale*/
    char* total_score = Receive_Message(client_fd,msg_type);
    /*COMUNICO A VIDEO IL PUNTEGGIO TOTALE*/
    SYSC(retvalue,write(STDOUT_FILENO,total_score,strlen(total_score)),"nella comunicazione del punteggio totale");
    return;
}