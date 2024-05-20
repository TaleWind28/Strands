#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "../Header/Trie.h"

void Caps_Lock(char* str){
    int retvalue;
    int i = 0;
    while (str[i] != '\0') {
        if (str[i] >= 'a' && str[i] <= 'z') {
            // Se il carattere è minuscolo, lo trasformo in maiuscolo
            str[i] = str[i] - 32;
        }
        i++;
    }
    return;
}

int main(){
    Trie*t; 
    t = create_node();
    char* stringa = strdup("ciao");
    Caps_Lock(stringa);
    insert_Trie(t,stringa);
    char buffer[100];
    Print_Trie(t,buffer,0);
    printf("%d\n",search_Trie("CIAO",t));
    
    return 0;
}