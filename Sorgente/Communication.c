#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "../Header/macro.h"
#include "../Header/Communication.h"

//ricevo un messaggio mandato tramite Send_Message
char* Receive_Message(int comm_fd,char* type){
    char* input;int retvalue,len;
    /*LEGGO I PRIMI DUE CARATTERI*/
    SYSC(retvalue,read(comm_fd,&len,sizeof(int)),"nella lettura della lunghezza del messaggio");
    /*MEMORIZZO IL TIPO DEL MESSAGGIO*/
    SYSC(retvalue,read(comm_fd,type,sizeof(char)),"nella lettura del messaggio");
    /*CONTROLLO CHE IL PAYLOAD NON VADA IGNORATO*/
    if (len == 0)return "stringa vuota";
    /*ALLOCO LA STRINGA CONTENTE IL PAYLOAD*/
    input = (char*)malloc(len+1);
    /*SCRIVO IL PAYLOAD SUL FILE DESCRIPTOR*/
    SYSC(retvalue,read(comm_fd,input,len),"nella lettura del messaggio");
    input[len] = '\0';
    return input;
}

//Invio un messaggio
void Send_Message(int comm_fd,char* payload,char type){
    int retvalue;int len = strlen(payload);
    /*COMUNICO LA LUNGHEZZA DEL MESSAGGIO CHE STO MANDANDO*/
    SYSC(retvalue,write(comm_fd,&len,sizeof(int)),"nella scrittura della lunghezza del messaggio");
    //usleep(10);//serve per sincornizzare read e write;
    /*COMUNICO IL TIPO DI MESSAGGIO CHE STO MANDANDO*/
    SYSC(retvalue,write(comm_fd,&type,sizeof(char)),"nella scrittura del tipo di messaggio");
    /*CONTROLLO DI AVERE QUALCOSA DA SCRIVERE SUL PAYLOAD*/
    if (len == 0)return;
    //usleep(10);//sincronizzo read e write
    /*MANDO IL PAYLOAD*/
    SYSC(retvalue,write(comm_fd,payload,len),"nella scrittura del payload");
    return;
}

void Caps_Lock(char* string){
    //recupero la lunghezza della stringa
    int len = strlen(string);
    //ciclo sulla stringa
    for(int i =0;i<len;i++){
        //controllo se il carattere è in lower case
        if (string[i]>= 'a' && string[i]<= 'z'){
            //lo porto in uppercase
            string[i]-= 32;
        }
    }
    return;
}