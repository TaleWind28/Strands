#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "../Header/macro.h"
#include "../Header/Communication.h"

char* Receive_Message(int comm_fd,char* type){
    char* input;int retvalue,len;
    /*LEGGO I PRIMI DUE CARATTERI*/
    SYSC(retvalue,read(comm_fd,&len,sizeof(int)),"nella lettura della lunghezza del messaggio");
    /*MEMORIZZO IL TIPO DEL MESSAGGIO*/
    SYSC(retvalue,read(comm_fd,type,sizeof(char)),"nella lettura del messaggio");
    /*CONTROLLO CHE IL PAYLOAD NON VADA IGNORATO*/
    if (len ==0)return "stringa vuota";
    /*ALLOCO LA STRINGA CONTENTE IL PAYLOAD*/
    input = (char*)malloc(len);
    /*SCRIVO IL PAYLOAD SUL FILE DESCRIPTOR*/
    SYSC(retvalue,read(comm_fd,input,len),"nella lettura del messaggio");
    return input;
}

void Send_Message(int comm_fd,char* payload,char type){
    int retvalue;int len = strlen(payload);
    /*COMUNICO LA LUNGHEZZA DEL MESSAGGIO CHE STO MANDANDO*/
    SYSC(retvalue,write(comm_fd,&len,sizeof(int)),"nella scrittura della lunghezza del messaggio");
    usleep(3);//serve per sincornizzare read e write;
    /*COMUNICO IL TIPO DI MESSAGGIO CHE STO MANDANDO*/
    SYSC(retvalue,write(comm_fd,&type,sizeof(char)),"nella scrittura del tipo di messaggio");
    /*CONTROLLO DI AVERE QUALCOSA DA SCRIVERE SUL PAYLOAD*/
    if (len == 0)return;
    usleep(3);//sincronizzo read e write
    /*MANDO IL PAYLOAD*/
    SYSC(retvalue,write(comm_fd,payload,len),"nella scrittura del payload");
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

void Recognize_Message(char type){
    if (type == MSG_OK)return;
    if (type == MSG_ERR);
    if (type == MSG_REGISTRA_UTENTE);
    if (type == MSG_MATRICE);
    if (type == MSG_TEMPO_PARTITA);
    if (type == MSG_TEMPO_ATTESA);
    if (type == MSG_PAROLA);
    if (type == MSG_PUNTI_FINALI);
    if (type == MSG_PUNTI_PAROLA);
}