#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "../Header/macro.h"
#include "../Header/Communication.h"

/*ASPETTO UN MESSAGGIO*/
char* Receive_Message(int Communication_fd,char Message_Type){
    int retvalue;char buffer[buff_size];
    /*leggo il messaggio sul file descriptor e lo salvo nel buffer*/
    SYSC(retvalue,read(Communication_fd,buffer,buff_size),"nella lettura del messaggio");
    /*tokenizzo la stringa per ottenere i campi significativi del messaggio*/
    char * token = strtok(buffer,",");
    // writef(retvalue,token);
    // writef(retvalue,"\n");
    /*salvo il tipo del messaggio*/
    Message_Type = token[0];
    token = strtok(NULL,",");
    /*recupero la lunghezza dei dati passati sul file descriptor*/
    int len = atoi(token);
    // writef(retvalue,token);
    // writef(retvalue,"\n");
    token = strtok(NULL,",");
    /*alloco una stringa dove memorizzare il campo data del messaggio e ce lo copio*/
    char* input = (char*)malloc(len*sizeof(char));
    strcpy(input,token);
    // writef(retvalue,token);
    // writef(retvalue,"\n");
    /*ritorno i dati passati*/
    return input;
}

/*INVIO UN MESSAGGIO*/
void Send_Message(int Communication_fd, char* data_to_send,char Message_Type){
    int retvalue,len = strlen(data_to_send);
    char* message = (char*)malloc((len+4)*sizeof(char));
    writef(retvalue,data_to_send);
    sprintf(message, "%c,%d,%s", Message_Type,len, data_to_send);
    writef(retvalue,message);
    SYSC(retvalue,write(Communication_fd,message,strlen(message)),"nella scrittura del messaggio");
    return;
}

// /*IDENTIFICO IL TIPO DI UN MESSAGGIO*/
// void Recognize_Message(int Communication_fd, char* data_to_send,char Message_Type){
    
//     return 0;
// }