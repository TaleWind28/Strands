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
    int retvalue;
    /*caso base*/
    if(wl == NULL)return -1;
    /*CONTROLLO DI AVER TROVATO LA PAROLA CHE CERCAVO*/
    if (strcmp(wl->word,word)==0) return 0;

    if (wl->next == NULL)return -1;
    writef(retvalue,"lista nulla");
    
    
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

// Funzione hash semplice (djb2)
unsigned int hash(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % TABLE_SIZE;
}

// Funzione per inizializzare la hash table
void init_table(Hash_Entry *table) {
    int retvalue;
    writef(retvalue,"ciao");
    char mess[1002];
    sprintf(mess,"%ld",TABLE_SIZE);
    writef(retvalue,mess);
    for (int i = 0; i < TABLE_SIZE; i++) {
        table[i].string = NULL;
        table[i].is_occupied = 0;
    }
    writef(retvalue,"ciao\n");
}

// Funzione per inserire una stringa nella hash table usando linear probing
void insert_string(Hash_Entry *table, const char *str) {
    unsigned int index = hash(str);
    unsigned int original_index = index;
    while (table[index].is_occupied) {
        index = (index + 1) % TABLE_SIZE;
        if (index == original_index) {
            printf("Hash table is full\n");
            return;
        }
    }
    table[index].string = strdup(str);  // Copia della stringa
    table[index].is_occupied = 1;
}

// Funzione per cercare una stringa nella hash table
int search_string(Hash_Entry *table, const char *str) {
    unsigned int index = hash(str);
    unsigned int original_index = index;
    while (table[index].is_occupied) {
        if (strcmp(table[index].string, str) == 0) {
            return index;
        }
        index = (index + 1) % TABLE_SIZE;
        if (index == original_index) {
            return -1;
        }
    }
    return -1;
}

void Delete_Table(Hash_Entry* hash_table){
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (hash_table[i].string) {
            free(hash_table[i].string);
        }
    }
    free(hash_table);
    return;
}