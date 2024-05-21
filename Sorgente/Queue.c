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
void WL_Push(Word_List* wl,char* word){
    Word_Node* el = (Word_Node*)malloc(sizeof(Word_Node));
    
    el->word = (char*)malloc(strlen(word)+1);
    //printf("%s",word);
    strcpy(el->word,word);
    /*faccio puntare l'elemento alla testa della lista*/
    el->next = *wl;
    /*faccio puntare la testa della lista all'elemento*/
    *wl= el;
    return;
}

/*ESTRAGGO L'ELEMENTO IN TESTA ALLA LISTA*/
char* WL_Pop(Word_List* wl){
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
int WL_Size(Word_List wl){
    /*caso base*/
    if(wl == NULL )return 0;
    /*chiamata ricorsiva*/
    return 1+WL_Size(wl->next);
}

/*COMUNICO LA TESTA DELLA LISTA*/
char* WL_Peek(Word_List wl){
    if (wl == NULL) return "lista vuota";
    return wl->word;
}

int WL_Find_Word(Word_List wl,char* word){
    //int retvalue;
    /*caso base*/
    if(wl == NULL)return -1;
    /*CONTROLLO DI AVER TROVATO LA PAROLA CHE CERCAVO*/
    if (strcmp(wl->word,word)==0) return 0;

    //if (wl->next == NULL)return -1;
    
    
    /*chiamata ricorsiva*/
    return WL_Find_Word(wl->next,word);
}

int WL_Splice(Word_List* wl,char* word){
    int retvalue;
    if (wl == NULL )return -1;
    Word_Node* prev = NULL;
    Word_Node* current = *(wl);
    //controllo se il prossimo nodo è quello che cerco
    while(current !=NULL && strcmp(current->word,word)!=0){
        prev = current;
        current = current->next;
    }
        if (current != NULL) {
        // Se è il primo nodo
            if (prev == NULL) {
                (*wl) = (*wl)->next;
            } else {
                prev->next = current->next;
            }
        // Libera la memoria occupata dalla stringa
        free(current->word);
        free(current);
    }
    return 0;
}
/*STAMPO LA LISTA*/
int Print_WList(Word_List wl){
    /*caso base*/
    if (wl == NULL) return 0;
    int retvalue;
    char print[buff_size];
    /*stampo l'elemento*/
    sprintf(print,"%s\n",wl->word);
    writef(retvalue,print);
    /*chiamata ricorsiva*/
    return Print_WList(wl->next);
}

