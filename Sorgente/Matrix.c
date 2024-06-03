#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "../Header/macro.h"
#include "../Header/Matrix.h"
#include "../Header/Queue.h"
#include "../Header/Communication.h"

/*CREA UNA MATRICE DATI IN INPUT IL NUMERO DI RIGHE E COLONNE E LA RITORNO*/
Matrix Create_Matrix(int rows, int columns){
    /*ALLOCO LA MATRICE CREANDO TANTI ARRAY DI CARATTERI QUANTE RIGHE*/
    char** matrix = (char**)malloc(rows*sizeof(char*));
    int size = rows*columns;
    
    /*ITERO SULLE RIGHE E PER OGNI RIGA ALLOCO UNA STRINGA CONTENENTE 1 SOLO CARATTERE*/
    int k = 0;
    
    for(int i = 0;i<rows;i++){
        for(int j = 0;j<columns;j++){
            matrix[i] = (char*)malloc(columns*(sizeof(char)));
            k++;
        }
    }
    

    /*MEMORIZZO LA MATRICE NELLA STRUTTURA DATI CREATA APPOSITAMENTE PER FACILITARE IL SUO UTILIZZO*/
    Matrix m = {rows,columns,matrix,rows*columns};
    /*INVOCO LA FUNZIONE ZEROS PER INIZIALIZZARE LA MATRICE*/
    zeros(m);

    return m;
}

/*INIZIALIZZA UNA MATRICE DI CARATTERI CON '0'*/
void zeros(Matrix m){
    for(int i = 0;i<m.rows;i++){
        for(int j = 0;j<m.columns;j++){
            m.matrix[i][j] = '0';
        }
    }
    return;
}

/*RIEMPIO TUTTA LA MATRICE IN BASE ALLA STRINGA PASSATA*/
void Fill_Matrix(Matrix m,char* letters){
    /*ALLOCO UNA STRINGA PER SCORRERE I CARATTERI DI QUELLA PRINCIPALE*/
    char* filling_row = (char*)malloc(m.columns*sizeof(char));
    /*INDICI DEL CICLO*/
    int i =0;int j = 0;
    /*ITERO SU TUTTE LE RIGHE DELLA MATRICE*/
    while(i<m.rows){
        /*RIEMPIO FILLING ROW DI UN NUMERO DI ELEMENTI PARI ALLE COLONNE DELLA MATRICE*/
        for(int i = 0;i<m.columns;i++){
            filling_row[i] = letters[j];
            j++;
        }
        //invoco fill row
        Fill_Matrix_Row(m,i,filling_row);
        i++;
    }
    return;
}

/*RIEMPIE UNA RIGA DELLA MATRICE*/
void Fill_Matrix_Row(Matrix m,int row,char* letter){
    for(int i = 0;i<m.columns;i++){
        m.matrix[row][i] = letter[i];
    }
    return;
}

/*RENDO LA MATRICE UNA STRINGA PER SEMPLIFICARE LE COMUNICAZIONI*/
char* Stringify_Matrix(Matrix m){
    /*INDICE PER SCORRERE LA STRINGA DOVE AVRO LA MATRICE*/
    int k = 0;
    char* string =(char*)malloc(m.size);
    for(int i =0;i<m.rows;i++){
        for(int j = 0;j<m.columns;j++){
            //printf("carattere:%c\n",m.matrix[i][j]);
            string[k] = m.matrix[i][j];
            k++;
        }
    }
    return string;
}

/*CARICO NELLA MATRICE IL CONTENUTO DI UNA RIGA DI UN FILE*/
void Load_Matrix(Matrix m, char* path_to_file,char exception,int* offset){
    int fd,retvalue;char* buffer;
    
    /*APRO IL FILE IN BASE AL PATH PASSATO*/
    SYSC(fd,open(path_to_file,O_RDONLY),"nell'apertura del file");
    
    /*LEGGO LA PRIMA RIGA DEL FILE*/
    buffer = File_Read(fd,exception,offset);
    Caps_Lock(buffer);
    /*AGGIORNO LA MATRICE IN BASE ALLA LETTURA*/
    
    //writef(retvalue,buffer);
    Adjust_String(buffer,exception);
    Fill_Matrix(m,buffer);
    /*CHIUDO IL FILE*/
    SYSC(retvalue,close(fd),"nella chiusura del file descriptor");
    return;
}

 char* File_Read(int fd, char exception, int* offset){
    //18 è la lunghezza massima perchè 16 parole +\n +\0
    char* buffer = (char*)malloc(17);
    int retvalue;char carattere;ssize_t n_read;
    /*mi posizioni nell'offset fornito*/
    SYSC(retvalue,lseek(fd,*offset,SEEK_SET),"nel setting dell'offset");
    /*leggo i caratteri della mia matrice*/
    for(int i =0;i<19;i++){
        /*leggo carattere per carattere*/
        SYSC(n_read,read(fd,&carattere,sizeof(char)),"nella lettura del carattere");
        if (n_read == 0){
            SYSC(*offset,lseek(fd,0,SEEK_SET),"nel resettare l'offset");
            break;
        }
        /*se trovo il \n esco*/
        if (carattere == '\n') break;
        Caps_Lock(&carattere);
        /*se trovo l'eccezione la skippo*/
        
        if (carattere==exception){
            buffer[i] = '?';
            SYSC(retvalue,read(fd,&carattere,sizeof(char)),"nella lettura dopo l'eccezione");
            continue;
        }
        buffer[i] = carattere;
        //printf("buffer:%s\n",buffer);
    }
    SYSC(*offset,lseek(fd,0,SEEK_CUR),"nella memorizzazione dell'offset");
    /*ritorno il buffer*/
    return buffer;
}

void Adjust_String(char* string,char x){
    int len = strlen(string);
    int j = 0;
    for(int i =0;i<len;i++){
        if (string[i] != x){
            string[j++] = string[i];
        }
    }
    string[j] = '\0';
    return;
}

int Is_Reachable(int* old_pos,int* pos){
    if (old_pos[0] == -1) return 0;
    /*stessa riga*/
    if (((old_pos[1]+1 == pos[1]) || (old_pos[1]-1 == pos[1]))&&(old_pos[0] == pos[0])){
        printf("STESSA RIGA\n");
        printf("riga:%d,colonna:%d\n",old_pos[0],old_pos[1]);
        return 0;
    }
    /*sono sulla stessa colonna,altrimenti mi sto muovendo in diagonale*/
    if(((old_pos[0]+1 == pos[0]) || (old_pos[0]-1 == pos[0]))&&(old_pos[1] == pos[1])){
        printf("STESSA COLONNA\n");
        printf("riga:%d,colonna:%d\n",old_pos[0],old_pos[1]);
        return 0;
    }
    /*sono in una posizione illegale*/
    return -1;
}

Position_List Position_List_Build(char* string,int NUM_ROW,int NUM_COL){
    Position_List pl = NULL;
    int k =0;
    for(int i =0;i<NUM_ROW;i++){
        for(int j = 0;j<NUM_COL;j++){
            Position_List_Push(&pl,string[k],i,j);
            k++;
        }
    }
    //Print_Position_List(pl);
    return pl;
}


/*INSERISCO UN ELEMENTO IN TESTA ALLA LISTA*/
void Position_List_Push(Position_List* cl,char x,int r, int c){
    Position_Node* el = (Position_Node*)malloc(sizeof(Position_Node));
    el->row = r;
    el->col = c;
    el->used = 0;
    el->val = x;
    /*faccio puntare l'elemento alla testa della lista*/
    el->next = *cl;
    /*faccio puntare la testa della lista all'elemento*/
    *cl= el;
    return;
}

int Is_Composable(Position_List pl, char* string){
    int i = 0;
    for(;i<strlen(string)-1;i++){
        int found=0,skip=0;
        int old_max = Find_Max_Occ(pl,string[i]);
        int new_max = Find_Max_Occ(pl,string[i+1]);
        for(int j=0;j<old_max;j++){
            int* old_pos = Find_Pos(pl,string[i],skip);
            //printf("old_char:%c\tr:%d,c:%d\n",string[i],old_pos[0],old_pos[1]);
            int skip2=0;
            for(int k=0;k<new_max;k++){
                int* new_pos = Find_Pos(pl,string[i+1],skip2);
                printf("new_char:%c\tr:%d,c:%d\n",string[i+1],new_pos[0],new_pos[1]);
                if (Is_Reachable(old_pos,new_pos)==0){
                    found = 1;
                    break;
                }else skip2++;
            }
            if (found == 1){ printf("TROVATO!:%c\n",string[i+1]);break;}
            else{ printf("RITENTA\n");skip++;} 
        }
        printf("found:%d\ni:%d,strlen:%d\n",found,i,strlen(string)-2);   
        if (found == 1 && i == (strlen(string)-2)){
            return 0;
        }    

    }
    //Print_Position_List(pl);
    return -1;
}

int* Find_Pos(Position_List pl, char x, int skip){
   
    if (pl == NULL){
        int* arr = (int*)malloc(2*sizeof(int));
        arr[0] = -1;
        arr[1] = -1;
        return arr;
    }
    //printf("%c %c %d\n",pl->val,x,skip);
    if (pl->val == x){
        printf("val:%c, pl->val:%c\n",x,pl->val);
        if (skip==0){
            int* arr = (int*)malloc(2*sizeof(int));
            arr[0] = pl->row;
            arr[1] = pl->col;
            return arr;
        }else{
            skip--;
        }
    }
    
    return Find_Pos(pl->next,x,skip);
}

int Find_Max_Occ(Position_List pl, char x){
    if (pl == NULL)return 0;
    if (pl->val == x)return 1+Find_Max_Occ(pl->next,x);
    else return Find_Max_Occ(pl->next,x);
}

/*ESTRAGGO L'ELEMENTO IN TESTA ALLA LISTA*/
void Position_List_Pop(Position_List* pl){
    /*creo un nodo temporaneo*/
    Position_Node* temp = *pl;
    /*faccio puntare la testa della lista al prossimo elemento*/
    *pl = (*pl)->next;
    free(temp);

    return;
}

/*CONTO GLI ELEMENTI DELLA LISTA*/
int Position_List_Size(Position_List cl){
    if (cl == NULL)return 0;
    return 1+Position_List_Size(cl->next);
}

/*COMUNICO LA TESTA DELLA LISTA*/
int* Position_List_Peek(Position_List cl){
    if (cl == NULL)return 0;
    int* head = (int*)malloc(2*sizeof(int));
    head[0] = cl->row;
    head[1] = cl->col;
    return head;
}

// /*CERCO UN ELEMENTO ALL'INTERNO DELLA LISTA*/
// int* Position_List_Find(Position_List cl,char x,int skip){
//     if(cl == NULL){
//         int * arr = (int*)malloc(2*sizeof(int));
//         arr[0] = -1;
//         arr[1] = -1;
//         return -1;
//     }
//     if(cl->val == x && skip == 0){
//         int * arr = (int*)malloc(2*sizeof(int)); 
//         arr[0] = cl->row;
//         arr[1] = cl->col;
//     }
//     skip--;
//     return Position_List_Find(cl->next,x,skip);
// }

/*CANCELLO UNA LISTA*/
int Delete_Position_List(Position_List* l){
    if (*l == NULL) return 0;
    if ((*l)->next == NULL){
        Position_List_Pop(l);
        return 0;
        }
    Position_Node* temp = *l;
    *l = (*l)->next;
    free(temp);
    return Delete_Position_List(l);
}

/*FUNZIONI DI STAMPA*/

/*STAMPO LA LISTA*/
int Print_Position_List(Position_List cl){
    if (cl == NULL) return 0;
    printf("val.%c,r:%d,c:%d\n",cl->val,cl->row,cl->col);
    return Print_Position_List(cl->next);
}


/*STAMPA LA MATRICE*/
void Print_Matrix(Matrix m,char special, char exception){
    int retvalue;char message[buff_size];
    
    for(int i =0;i<m.rows;i++){
        for(int j = 0;j<m.columns;j++){
            /*SCRIVO IN UN BUFFER L'ELEMENTO DELLA MATRICE*/
            if (m.matrix[i][j] == special){
                sprintf(message,"%c%c\t",exception,'U');
            }
            else{
                sprintf(message,"%c\t", m.matrix[i][j]);
            }
            /*SCRIVO A VIDEO L'ELEMENTO*/
            writef(retvalue,message);
        }
        /*PASSO ALLA PROSSIMA RIGA*/
        writef(retvalue,"\n");
    }

    return;
}