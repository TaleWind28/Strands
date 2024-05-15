#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "../Header/macro.h"
#include "../Header/Queue.h"

/*INSERISCO UN ELEMENTO IN TESTA ALLA LISTA*/
void Push(Word_List* wl,char* word){
    Word_Node* el = (Word_Node*)malloc(sizeof(Word_Node));
    
    el->word = (char*)malloc(strlen(word)*sizeof(char));
    //printf("%s",word);
    strcpy(el->word,word);
    /*faccio puntare l'elemento alla testa della lista*/
    el->next = *wl;
    /*faccio puntare la testa della lista all'elemento*/
    *wl= el;
    return;
}

/*ESTRAGGO L'ELEMENTO IN TESTA ALLA LISTA*/
char* Pop(Word_List* wl){
    if (wl == NULL) return "lista vuota";
    /*copio la stringa del nodo in testa per restituirla*/ 
    char* tword = (*wl)->word;
    /*creo un nodo temporaneo*/
    Word_Node* temp = *wl;
    /*faccio puntare la testa della lista al prossimo elemento*/
    *wl = (*wl)->next;
    /*dealloco il nodo temporaneo*/
    free(temp);

    return tword;
}

/*CONTO GLI ELEMENTI DELLA LISTA*/
int Size(Word_List wl){
    /*caso base*/
    if(wl == NULL )return 0;
    /*chiamata ricorsiva*/
    return 1+Size(wl->next);
}

/*COMUNICO LA TESTA DELLA LISTA*/
char* Peek(Word_List wl){
    if (wl == NULL) return "lista vuota";
    return wl->word;
}

int Find_Word(Word_List wl,char* word){
    /*caso base*/
    if(wl == NULL)return -1;

    /*CONTROLLO DI AVER TROVATO LA PAROLA CHE CERCAVO*/
    if (strcmp(wl->word,word)==0) return 0;
    
    /*chiamata ricorsiva*/
    return Find_Word(wl->next,word);
}

/*STAMPO LA LISTA*/
int Print_WList(Word_List wl){
    /*caso base*/
    if (wl == NULL) return 0;

    /*stampo l'elemento*/
    printf("%s\n",wl->word);

    /*chiamata ricorsiva*/
    return Print_WList(wl->next);
}

