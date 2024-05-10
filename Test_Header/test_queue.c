#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "../Header/macro.h"
#include "../Header/Queue.h"

int main(int argc, char* argv[]){
    Word_List lista = NULL;
    Push(&lista,"ciao");
    Push(&lista,"cia");
    Push(&lista,"ci");
    Push(&lista,"c");
    Print_WList(lista);
    printf("size:%d\n",Size(lista));
    printf("peek:%s\n",Peek(lista));
    printf("%d\n",Find_Word(lista,"ciao"));
    printf("ho rimosso:%s\n",Pop(&lista));
    printf("ho rimosso:%s\n",Pop(&lista));
    printf("ho rimosso:%s\n",Pop(&lista));
    printf("%d\n",Find_Word(lista,"c"));
    printf("ho rimosso:%s\n",Pop(&lista));
    printf("%d\n",Find_Word(lista,"ciao"));
    return 0;
}