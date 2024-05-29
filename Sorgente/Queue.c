#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <pthread.h>
#include "../Header/macro.h"
#include "../Header/Queue.h"

/*INSERISCO UN ELEMENTO IN TESTA ALLA LISTA*/
void WL_Push(Word_List* wl,char* word){
    Word_Node* el = (Word_Node*)malloc(sizeof(Word_Node));
    //alloco spazio per la parola
    el->word = (char*)malloc(strlen(word)+1);
    //registro il thread che gestisce l'utente
    el->handler = pthread_self();
    //imposto a 0 il punteggio dell'utente
    el->points = 0;
    //printf("%s",word);
    strcpy(el->word,word);
    /*faccio puntare l'elemento alla testa della lista*/
    el->next = *wl;
    /*faccio puntare la testa della lista all'elemento*/
    *wl= el;
    return;
}

void WL_Push_Thread(Word_List* wl,char* word, int socket_fd){
    Word_Node* el = (Word_Node*)malloc(sizeof(Word_Node));
    //alloco spazio per la parola
    el->word = (char*)malloc(strlen(word)+1);
    //registro il thread che gestisce l'utente
    el->handler = pthread_self();
    //imposto a 0 il punteggio dell'utente
    el->points = 0;
    //imposto il scoket di comunicazione
    el->client_fd = socket_fd;
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

/*COMUNICO IL GESTORE DELLA TESTA DELLA LISTA*/
pthread_t WL_Peek_Hanlder(Word_List wl){
    if (wl == NULL)return -1;
    return wl->handler;
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

int WL_Update_Score(Word_List wl,pthread_t gestore,int new_score){
    //caso base
    if (wl == NULL) return 0;
    //caso base
    if(wl->handler == gestore){
        wl->points+= new_score;
        return 0;
    }
    //caso ricorsivo
    return WL_Update_Score(wl->next,gestore,new_score);
}
//ripensare
int WL_Retrieve_Score(Word_List wl,pthread_t gestore){
    //caso base
    if (wl == NULL)return -1;
    //caso base
    if (wl->handler == gestore){
        return wl->points;
    }
    //caso ricorsivo
    return WL_Retrieve_Score(wl->next,gestore);
}

char* WL_Retrieve_User(Word_List wl,pthread_t gestore){
    //caso base
    if (wl == NULL)return "";
    //caso base
    if (wl->handler == gestore){
        return wl->word;
    }
    //caso ricorsivo
    return WL_Retrieve_User(wl->next,gestore);
}

int WL_Splice(Word_List* wl,char* word){
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

//prendi il client_fd
int WL_Retrieve_Socket(Word_List wl,pthread_t gestore){
    if (wl == NULL)return -1;
    if(wl->handler == gestore)return wl->client_fd;
    return WL_Retrieve_Socket(wl->next,gestore);
}