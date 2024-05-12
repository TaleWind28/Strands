#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "../Header/macro.h"
#include "../Header/Communication.h"


#define PORT 2500

void server();
void client();

int main(){
    int retvalue;
    pid_t pid;
    SYSC(pid,fork(),"nella fork");
    if (pid == 0){
        /*sono il figlio*/
        client();
        //printf("ho scritto:%s\n",message);
        exit(EXIT_SUCCESS);
    }
    server();
    /*attesa del figlio*/
    SYSC(retvalue,wait(NULL),"nella wait");
    return 0;
}

void server(){
    /*dichiarazione variabili*/
    int retvalue, server_fd, client_fd;
    struct sockaddr_in server_address, client_address;
    socklen_t client_length = sizeof(client_address);

    /*inizializzazione server address*/
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    server_address.sin_port = htons(PORT); 
    
    /*creazione socket*/
    SYSC(server_fd,socket(AF_INET,SOCK_STREAM,0),"creazione socket del server");
    
    /*binding dell'indirizzo*/
    SYSC(retvalue,bind(server_fd,(struct sockaddr*)&server_address,sizeof(server_address)),"nella bind");
    
    /*predisposizione del listner*/
    SYSC(retvalue,listen(server_fd,10),"nella listen");
    
    /*accettazione delle richieste*/
    
    SYSC(client_fd,accept(server_fd,(struct sockaddr*)&client_address,&client_length),"nella accept");
    char msg_type = 'a';    
    char * message = Receive_Message(client_fd,msg_type);
    writef(retvalue,message);
    printf("%c",msg_type);
    /*chiusura dei socket*/
    SYSC(retvalue,close(client_fd),"chiusura client-fake");
    SYSC(retvalue,close(server_fd),"chiusura server");

    return;
}

void client(){
    /*dichiarazione ed inizializzazione variabili*/
    int retvalue, client_fd;
    struct sockaddr_in server_address;
    socklen_t server_len = sizeof(server_address);
    
    /*inizializzazione server_address*/
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    
    /*creo il socket per comunicare*/
    SYSC(client_fd,socket(AF_INET,SOCK_STREAM,0),"nella creazione del client");
        
    /*chiamo la connect*/
    SYSC(retvalue,connect(client_fd,(struct sockaddr*)&server_address,server_len),"nella connect");

    /*mando un messaggio al server*/
    char * message = "dioboia funziona\n";
    Send_Message(client_fd,message,MSG_OK);

    /*chiudo il socket*/
    SYSC(retvalue,close(client_fd),"chiusura del cliente");
    
    return;
}