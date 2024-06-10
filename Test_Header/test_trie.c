#define _XOPEN_SOURCE 700
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include "../Header/Trie.h"

#define DIZIONARIO "../Text/Dizionario.txt"

void Caps_Lock(char* str){
   int len = strlen(str);
   for(int i =0;i<len;i++){
        if(str[i]>='a' && str[i]<='z'){
            str[i]-=32;
        }
   }
   return;
}

void Load_Dictionary(Trie* Dictionary, char* path_to_dict){
    //APRO IL FILE TRAMITE IL PATH
    FILE* dict = fopen(path_to_dict,"r");
    //CREO UNA VARIABILE PER MEMORIZZARE LE PAROLE
    char word[256];
    //LEGGO TUTTO IL FILE
    while(fscanf(dict,"%s",word)!=EOF){
        //STANDARDIZZO LE PAROLE DEL DIZIONARIO METTENDOLE IN UPPERCASE
        Caps_Lock(word);
        //INSERISCO LA PAROLA NEL TRIE
        //int retvalue;
        //printf("%s\n",word);
        
        insert_Trie(Dictionary,word);
    }
    return;
}
 Trie* t;
int main() { 
    t = create_node();
    Load_Dictionary(t,DIZIONARIO);
    //CARICO IL DIZIONARIO IN MEMORIA

    char bufer[256];
    write(1, "inserito tutto\n", 15);
    Print_Trie(t,bufer,0);
    for(int i =0;i< 5;i++){
        char input[256];
        write(STDOUT_FILENO,"cerca una parola sul Trie\n",27);
        read(STDIN_FILENO,input,256);
        char* token = strtok(input,"\n");
        Caps_Lock(token);
    //printf("%d",search_Trie("CASI",t));
        if(search_Trie(token,t)==0){
            printf("PAROLA PRESENTE NEL TRIE\n");
        }else{
            printf("PAROLA NON PRESENTE NEL TRIE\n");
        }
    }
    return 0;
}