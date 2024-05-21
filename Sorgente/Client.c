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
#define HELP_MESSAGE "Per prima cosa se non lo hai già fatto resgistrati mediante il comando registra_utente seguito dal tuo nome, poi potrai usare i seguenti comandi:\np seguito da una parola per indovinare una parola presente nella matrice che vedi a schermo\n matrice per visualizzare a schermo la matrice ed il tempo residuo di gioco\naiuto che ti mostra i comandi a te disponibili\nfine che ti fa uscire dalla partita in corso\n"
void Play(int client_fd);


Matrix matrice_player;

int take_action(char* input, int comm_fd);

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
    //char type = '0';
    char input_buffer[buff_size];
    ssize_t n_read;
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
    int retvalue;
    char type = MSG_ERR;
    char* token = strtok(input, " ");
    switch(input[0]){
        case 'a':
            writef(retvalue,HELP_MESSAGE);
            break;
        case 'r': 
            
            token = strtok(NULL," ");
            if (token == NULL || token[0] == '\n'){
                writef(retvalue,"nome utente non valido\n");
                break;
            }
            //writef(retvalue,token);
            while(type!= MSG_OK){
                Send_Message(comm_fd,token,MSG_REGISTRA_UTENTE);
                char* answer = Receive_Message(comm_fd,&type);
                writef(retvalue,answer);
                free(answer);
            }
            break;
        case 'm':
            Send_Message(comm_fd,"matrice",MSG_MATRICE);
            char* matrice = Receive_Message(comm_fd,&type);
            //writef(retvalue,matrice);
            matrice_player = Create_Matrix(4,4);
            Fill_Matrix(matrice_player,matrice);
            Print_Matrix(matrice_player,4,4);
            free(matrice);
            break;
        case 'p':
            token = strtok(NULL,"\n");
            if (token == NULL || token[0] == '\n'){
                Send_Message(comm_fd,"lol\n",MSG_PAROLA);
            }else{
                Caps_Lock(token);
                Send_Message(comm_fd,token,MSG_PAROLA);
            }
            char* answer = Receive_Message(comm_fd,&type);
            writef(retvalue,answer);
            free(answer);
            break;
        case 'f':
            Send_Message(comm_fd,"fine",MSG_CHIUSURA_CONNESSIONE);
            return -1;
            break;
        default:
            writef(retvalue,"comando non disponibile, digitare aiuto per una lista dettagliata\n");
            break;
    }   
    return 0;
}