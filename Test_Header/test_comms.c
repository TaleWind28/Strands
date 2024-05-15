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
    //char buffer[buff_size];
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
    
    ssize_t n_read;
    char* buffer = (char*)malloc(buff_size*sizeof(char));
    //mess.Data = (char*)malloc(buff_size*sizeof(char));
    for(int i=0;i<2;i++){
        
        SYSC(n_read,read(client_fd,buffer,buff_size),"nella read");
        //writef(retvalue,mex);
        char tok[n_read];
        strncpy(tok,buffer,n_read);
        char* message = (char*)malloc((n_read-5)*sizeof(char));
        int len = 0;
        writef(retvalue,"tok:\n");
        writef(retvalue,tok);
        writef(retvalue,"\n");
        Decompose_Message(tok,message,msg_type,len);
        //realloc(buffer,buff_size*sizeof(char));
        //writef(retvalue,message);
    }
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

    char * message2 = "dioboia\n";
    Send_Message(client_fd,message2,MSG_OK);

    /*chiudo il socket*/
    SYSC(retvalue,close(client_fd),"chiusura del cliente");
    
    return;
}