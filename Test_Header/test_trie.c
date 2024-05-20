#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "../Header/Trie.h"


int main(){
    Trie*t = create_node();
    insert_Trie(t,"TORRE");
    insert_Trie(t,"CIAO");
    insert_Trie(t,"C");
    insert_Trie(t,"CI");
    insert_Trie(t,"CIA");
    insert_Trie(t,"CIMA");
    insert_Trie(t,"COMA");
    printf("%d\n",search_Trie("COMA",t));
    char buffer[100];
    Print_Trie(t,buffer,0);
    return 0;
}