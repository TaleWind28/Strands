#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "../Header/macro.h"
#include "../Header/Matrix.h"

int main(int argc,char* argv[]){
    if(argc!=5){
        perror("usare la seguente sintassi: nome_programma rows columns contenuto_matrice word");
        exit(EXIT_FAILURE);
    }
    int rows = atoi(argv[1]);
    int columns = atoi(argv[2]);
    char* word = argv[4];
    Matrix matrix = Create_Matrix(rows,columns);
    Fill_Matrix(matrix,argv[3]);
    //Print_Matrix(matrix);
    
    char* string = (char*)malloc(matrix.size*(sizeof(char)));
    Stringify_Matrix(matrix,string);
    
    //sprintf("stringa matriciale indotta: %s\n",string);
    
    //Fill_Matrix(matrix,string);
    Build_Charmap(matrix);
    
    //Print_Matrix(matrix);

    /*Test Find_Element*/
    //pos = Find_Charmap_Element(matrix,'a');
    //printf("posizione elemento a:%d %d\n",pos[0],pos[1]);
    //char carattere = 'l';
    //Charmap old_pos  = Find_Charmap_Element(matrix,carattere);
    //Charmap next_pos = Find_Charmap_Element(matrix,'o');
    //Print_FCE(old_pos,carattere);
    //Print_FCE(next_pos,'o');
    //Print_CharMap(matrix.map);
    //int res1 = 3;
    // int res1 = Is_Composable(matrix,word);
    // if (res1 == 0){
    //     int res = Validate_Word(matrix,matrix.map,word);
    //     printf("validate:%d\n",res);
    // }
    // Position_List l = NULL;
    // Position_List_Push(&l,0,0);
    // Position_List_Push(&l,1,0);
    // Position_List_Push(&l,1,1);
    // Delete_Position_List(&l);
    // Print_Position_List(l);
    printf("nuova validate:%d\n",Validate(matrix,word));
    //printf("aaaaaa\n");
    // char message[buff_size];
    // sprintf(message,"%d\n",old_pos[0][0]);
    // writef(retvalue,message);
    
    // old_pos = Find_Charmap_Element(matrix,'v');
    // old_pos = Find_Charmap_Element(matrix,'e');
    // old_pos = Find_Charmap_Element(matrix,'c');
    // old_pos = Find_Charmap_Element(matrix,'l');
    // old_pos = Find_Charmap_Element(matrix,'i');
    // old_pos = Find_Charmap_Element(matrix,'h');
    // old_pos = Find_Charmap_Element(matrix,'r');
    // old_pos = Find_Charmap_Element(matrix,'a');
    // old_pos = Find_Charmap_Element(matrix,'t');
    // old_pos = Find_Charmap_Element(matrix,'u');

    // /*Test Is_Reachable*/
    // printf("la nuova posizione è raggiungibile?:%d\n",Is_Reachable(matrix,old_pos,pos));
    // printf("%d\n",Validate_Word(matrix,"olive"));//output atteso: valida -> 0;

    exit(0);
}