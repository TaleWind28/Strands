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
    writef(retvalue,"connesso\n");
    char type = '0';
    
    Send_Message(client_fd,"ciao",MSG_REGISTRA_UTENTE);
    char * response = Receive_Message(client_fd,&type);

    Send_Message(client_fd,"ciao",MSG_REGISTRA_UTENTE);
    response = Receive_Message(client_fd,&type);

    Send_Message(client_fd,"matrice",MSG_MATRICE);
    response = Receive_Message(client_fd,&type);
    Matrix mat = Create_Matrix(4,4);
    Fill_Matrix(mat,response);
    Print_Matrix(mat,4,4);
    char input[256];
    ssize_t lettura = read(STDIN_FILENO,input,256);
    char* input_tok = (char*) malloc(lettura-1);
    if (lettura <3)strcpy(input,"lol\n");
    input_tok = strtok(input,"\n");
    Caps_Lock(input_tok);
    printf("strlen:%d\n",strlen(input_tok));
    //write(STDOUT_FILENO,&len,sizeof(int));
    Send_Message(client_fd,input_tok,MSG_PAROLA);
    response = Receive_Message(client_fd,&type);
    writef(retvalue,response);
    /*messaggio fine del client*/
    Send_Message(client_fd,"ciao",MSG_CHIUSURA_CONNESSIONE);
    
    /*chiudo il socket*/
    SYSC(retvalue,close(client_fd),"chiusura del cliente");
}