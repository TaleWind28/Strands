#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "../Header/macro.h"
#include "../Header/Trie.h"
#define MAX_CHAR 26

Trie* create_node(){
    //alloco il nodo
    Trie* trie = (Trie*)malloc(sizeof(Trie)); 
    //inizializzo i valori della radice
    trie->is_word = -1;
    //inizializzo i figli
    for (int i =0;i< NUM_CHAR;i++){
        trie->figli[i] = NULL;
    }
    return trie;
}

int search_Trie(char* word, Trie* trie){
    //caso base, il trie è vuoto
    if (trie == NULL)return -1;
    //sono arrivato in fondo alla parola
    if (*word == '\0'){
        return trie->is_word;
    }
    //calcolo la posizione del carattere da cercare
    int index = *word -'A';
    //se non esiste il figlio -> non è presente la parola
    if (trie->figli[index] == NULL){
        return -1;
    }
    //chiamata ricorsiva
    return search_Trie(word+1,trie->figli[index]);
}



int insert_Trie(Trie* root, char* word){
    //caso base
    if (root == NULL){
        root = create_node();
    }
    //caso base
    if (*word == '\0'){
        if (root->is_word == -1){
            root->is_word = 0;
            return 0;
        }
        return -1;
        
    }
    //cerco il prossimo figlio
    int target_child = *word - 'A';
    if (root->figli[target_child] == NULL){
        root->figli[target_child] = create_node();
    }
    //chiamata ricorsiva
    return insert_Trie(root->figli[target_child],word+1);
}


void Print_Trie(Trie* trie, char* buffer,int depth){
    //caso base
    if (trie == NULL){
        printf("l'albero è vuoto\n");
        return;
    }
    //caso base 
    if (trie->is_word == 0){
        buffer[depth] = '\0';
        printf("%s\n",buffer);
    }
    //faccio una ricerca per trovare il prossimo carattere su cui ricorrere
    for(int i = 0;i<NUM_CHAR;i++){
        if (trie->figli[i] != NULL){
            buffer[depth] = i + 'A';
            Print_Trie(trie->figli[i],buffer,depth+1);
        }
    }
    return;
}
