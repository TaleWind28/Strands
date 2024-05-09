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

#define NUM_ROWS 4
#define NUM_COLUMNS 4

void Play(int client_fd);


int main(int argc, char* argv[]){
    /*dichiarazione ed inizializzazione variabili*/
    int retvalue, client_fd;
    struct sockaddr_in server_address;
    socklen_t server_len = sizeof(server_address);
    /*INIZIALIZZAZIONE VARIABILI IN BASE ALL'ARGOMENTO DELLA RIGA DI COMANDO*/
    //char* HOST = argv[1];
    int PORT = atoi(argv[2]);

    /*CONTROLLO PARAMETRI RIGA DI COMANDO*/
    if(argc != 3){
        perror("usare la seguente sintassi: nome programa host porta_server");
        exit(EXIT_FAILURE);
    }
    /*inizializzazione server_address*/
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    // /*PROVA INTERNET*/
    // server_address.sin_addr.s_addr = inet_addr("192.168.228.171");
    // server_address.sin_port = htons(20000);
    
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
    int retvalue;char matrice[buff_size];
    /*LEGGO LA MATRICE PASSATA DAL SERVER*/
    SYSC(retvalue,read(client_fd,matrice,buff_size),"nella lettura della matrice");
    /*CREO LA MATRICE DA FAR UTILIZZARE AL CLIENT*/
    Matrix client_matrix = Create_Matrix(NUM_ROWS,NUM_COLUMNS);
    /*RIEMPIO LA MATRICE COI DATI DEL SERVER*/
    Fill_Matrix(client_matrix,matrice);
    /*COSTRUISCO LA MAPPA DEI CARATTERI DELLA MATRICE*/
    Build_Charmap(client_matrix);
    /*STAMPO LA MATRICE A SCHERMO*/
    Print_Matrix(client_matrix);
    writef(retvalue,"Inserisci una parola che hai letto nella matrice\t");
    ssize_t n_read;
    char score[buff_size];
    for(int i =0;i<5;i++){
        char** input = (char**)malloc(buff_size*(sizeof(char*)));
        input[i] =(char*)malloc(buff_size*(sizeof(char)));
        /*ASPETTO UN INPUT DALL'UTENTE*/
        SYSC(n_read,read(STDIN_FILENO,input[i],buff_size),"nella lettura dell'input del cliente");
        if(strlen(input[i]) == 1){
            input[i] = "lol\n";
            n_read = strlen(input[i]);
        }

        /*SCRIVO SUL FILE DESCRIPTOR CONDIVISO COL SERVER IL MESSAGGIO DA MANDARE*/
        SYSC(retvalue,write(client_fd,input[i],n_read),"nella write");
        /*ASPETTO CHE MI VENGA COMUNICATO IL PUNTEGGIO*/
        SYSC(n_read,read(client_fd,score,buff_size),"nella lettura del punteggio");
        /*SCRIVO A VIDEO IL PUNTEGGIO*/
        //writef(retvalue,"ok");
        SYSC(retvalue,write(STDOUT_FILENO,score,n_read),"nella comunicazione del punteggio a video");

    }
    SYSC(n_read,read(client_fd,score,buff_size),"nella lettura del punteggio totale");
    return;
}