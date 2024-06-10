#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "../Header/macro.h"
#include "../Header/Matrix.h"
#include "../Header/Communication.h"

#define ROWS 4
#define COLS 4
#define FILE_MATRIX "../Text/Matrici.txt"



int main(int argc,char* argv[]){
    int offset = 0;
    for(int i =0;i<3;i++){
        char* matrix  = Load_Matrix(FILE_MATRIX,'Q',&offset);
        Graph* g = Build_Graph(matrix,ROWS,COLS);

        Print_Matrix(matrix,ROWS,COLS,'?');
        int retvalue;char input_buffer[buff_size];
        SYSC(retvalue,read(STDIN_FILENO,input_buffer,buff_size),"nella lettura da stdin");
        char* token = strtok(input_buffer,"\n");
        printGraph(g);
        Caps_Lock(token);
        printf("%s\n",token);
        if (dfs(g,token)==true){
            printf("PAROLA PRESENTE!\n");
        }else{
            printf("LA PAROLA CHE CERCAVI NON È QUI\n");
        }
        //freeGraph(g);
        //free(matrix); 
    }
    return 0;
}