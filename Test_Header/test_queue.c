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
    WL_Push(&lista,"ciao");
    WL_Push(&lista,"cia");
    WL_Push(&lista,"ci");
    WL_Push(&lista,"c");
    Print_WList(lista);
    printf("size:%d\n",WL_Size(lista));
    printf("peek:%s\n",WL_Peek(lista));
    printf("%d\n",WL_Find_Word(lista,"ciao"));
    // printf("ho rimosso:%s\n",Pop(&lista));
    // printf("ho rimosso:%s\n",Pop(&lista));
    // printf("ho rimosso:%s\n",Pop(&lista));
    // printf("%d\n",Find_Word(lista,"c"));
    // printf("ho rimosso:%s\n",Pop(&lista));
    // printf("%d\n",Find_Word(lista,"ciao"));
    printf("%d\n",WL_Splice(&lista,"ci"));
    Print_WList(lista);
    return 0;
}