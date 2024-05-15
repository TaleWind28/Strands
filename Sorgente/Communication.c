#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "../Header/macro.h"
#include "../Header/Communication.h"

/*ASPETTO UN MESSAGGIO*/
char* Receive_Message(int Communication_fd,char Message_Type){
    ssize_t n_read;char* buffer = (char*)malloc(buff_size*sizeof(char));
    /*leggo il messaggio sul file descriptor e lo salvo nel buffer*/
    SYSC(n_read,read(Communication_fd,buffer,buff_size),"nella lettura del messaggio");
    buffer = realloc(buffer,n_read*sizeof(char));
    /*tokenizzo la stringa per ottenere i campi significativi del messaggio*/
    char * token = strtok(buffer,",");
    //writef(retvalue,token);
    /*salvo il tipo del messaggio*/
    Message_Type = token[0];
    token = strtok(NULL,",");
    //writef(retvalue,token);
    /*recupero la lunghezza dei dati passati sul file descriptor*/
    int len = strtol(token,NULL,10);
    token = strtok(NULL,",");
    //sprintf(message,"%c,%d,%s\n",Message_Type,len,token);
    //writef(retvalue,message);
    /*alloco una stringa dove memorizzare il campo data del messaggio e ce lo copio*/
    char* input = (char*)malloc(len*sizeof(char));
    strcpy(input,token);
    /*ritorno i dati passati*/
    return input;
}

/*INVIO UN MESSAGGIO*/
void Send_Message(int Communication_fd, char* data_to_send,char Message_Type){
    int retvalue,len = strlen(data_to_send);
    /*ALLOCO UNA VARIABILE PER COMPORRE IL MESSAGGIO DA MANDARE*/
    char* message = (char*)malloc((len+4)*sizeof(char));
    /*COMPONGO IL MESSAGGIO*/
    sprintf(message, "%c,%d,%s", Message_Type,len, data_to_send);
    /*SCRIVO IL MESSAGGIO SUL FILE DESCRIPTOR*/
    SYSC(retvalue,write(Communication_fd,message,strlen(message)),"nella scrittura del messaggio");
    free(message);
    return;
}

void Caps_Lock(char* string){
    int len = strlen(string);
    for(int i =0;i<len;i++){
        if (string[i]>= 'a' && string[i]<= 'z'){
            string[i]-= 'a' - 'A';
        }
    }
    return;
}
// /*IDENTIFICO IL TIPO DI UN MESSAGGIO*/
// void Recognize_Message(int Communication_fd, char* data_to_send,char Message_Type){
    
//     return 0;
// }
char message[buff_size];
int retvalue;
void Decompose_Message(char* message_to_decompose,char* message_decomposed,char decomposed_type,int decompose_len){
    char* token = strtok(message_to_decompose,",");
    writef(retvalue,message_to_decompose);
    decomposed_type = token[0];
    writef(retvalue,"strcpy\n");
    token = strtok(NULL,",");
    writef(retvalue,token);
    writef(retvalue,"strcpy\n");
    decompose_len = strtol(token,NULL,10);
    token = strtok(NULL,",");
    writef(retvalue,"strcpy\n");
    strncpy(message_decomposed,token,decompose_len);
    sprintf(message,"%c,%d,%s\n",decomposed_type,decompose_len,message_decomposed);
    writef(retvalue,message);
    return;
}