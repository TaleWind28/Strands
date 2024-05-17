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
    Charmap* map = (Charmap*)malloc(size*sizeof(Charmap));
    map->size = size;
    
    /*ITERO SULLE RIGHE E PER OGNI RIGA ALLOCO UNA STRINGA CONTENENTE 1 SOLO CARATTERE*/
    int k = 0;
    
    for(int i = 0;i<rows;i++){
        for(int j = 0;j<columns;j++){
            matrix[i] = (char*)malloc(columns*(sizeof(char)));
            map[k].carattere = '0';
            map[k].row =(int*)malloc(size*sizeof(int));
            map[k].column =(int*)malloc(size*sizeof(int));
            k++;
        }
    }
    

    /*MEMORIZZO LA MATRICE NELLA STRUTTURA DATI CREATA APPOSITAMENTE PER FACILITARE IL SUO UTILIZZO*/
    Matrix m = {rows,columns,matrix,rows*columns,map};
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
void Load_Matrix(Matrix m, char* path_to_file,char exception){
    int fd,retvalue;char buffer[buff_size];ssize_t n_read;
    /*APRO IL FILE IN BASE AL PATH PASSATO*/
    SYSC(fd,open(path_to_file,O_RDONLY),"nell'apertura del file");
    /*LEGGO LA PRIMA RIGA DEL FILE*/
    SYSC(n_read,read(fd,buffer,buff_size),"nella scrittura sul buffer");
    /*AGGIORNO LA MATRICE IN BASE ALLA LETTURA*/
    Caps_Lock(buffer);
    //writef(retvalue,buffer);
    Adjust_String(buffer,exception);
    Fill_Matrix(m,buffer);
    /*CHIUDO IL FILE*/
    SYSC(retvalue,close(fd),"nella chiusura del file descriptor");
    return;
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
    //char mess[buff_size];
    // int retvalue;
    // sprintf(mess,"stringa:%s,lunghezza:%d\n",string,strlen(string));
    // writef(retvalue,mess);
    return;
}

int Is_Reachable(Matrix m,int* old_pos,int* pos){
    if (old_pos[0] == -1) return 0;
    /*stessa riga*/
    if (((old_pos[1]+1 == pos[1]) || (old_pos[1]-1 == pos[1]))&&(old_pos[0] == pos[0])){
            //printf("riga:%d,colonna:%d\n",old_pos[0],old_pos[1]);
        return 0;
    }
    /*sono sulla stessa colonna,altrimenti mi sto muovendo in diagonale*/
    if(((old_pos[0]+1 == pos[0]) || (old_pos[0]-1 == pos[0]))&&(old_pos[1] == pos[1])){
        //printf("riga:%d,colonna:%d\n",old_pos[0],old_pos[1]);
        return 0;
    }
    /*sono in una posizione illegale*/
    return -1;
}

/*RIEMPIO LA MAPPATURA DEI CARATTERI DELLA MATRICE*/
void Build_Charmap(Matrix m){
    int k = 0;int inserted;
    /*CICLO SU RIGHE E COLONNE DELLA MATRICE*/
    for(int i =0;i<m.rows;i++){
        for(int j = 0;j<m.columns;j++){
            /*AZZERO LA VARIABILE CHE SEGNALA L'INSERIMENTO*/
            inserted = 0;
            /*CONTROLLO SE LA POSIZIONE ATTUALE È VUOTA*/
            if(m.map[k].carattere == '0'){
                /*STAMPA DI DEBUG*/
                //printf("caso 1:carattere:%c\n",m.matrix[i][j]);
                /*AGGIORNO IL CARATTERE*/
                m.map[k].carattere = m.matrix[i][j];
                /*INSERISCO LA POSIZIONE IN RIGHE E COLONNE IN BASE ALL'OCCORRENZA*/
                m.map[k].row[m.map[k].occorrenza] = i;
                m.map[k].column[m.map[k].occorrenza] = j;
                /*AGGIUNGO 1 ALL'OCCORRENZA*/
                m.map[k].occorrenza=1;;
                //printf("carattere:%c, occorrenza:%d, row:%d, col:%d\n",m.matrix[i][j],m.map[k].occorrenza,m.map[k].row[m.map[k].occorrenza],m.map[k].column[m.map[k].occorrenza]);

                /*RIFERISCO CHE LA POSIZIONE È STATA INSERITA*/
                inserted = 1;
            }else{
                /*CONTROLLO I CARATTERI GIà INSERITI*/
                for(int k1 = k;k1>=0;k1--){
                    /*CONTROLLO SE DEVO AUMENTARE L'OCCORRENZA*/
                    if(m.map[k1].carattere == m.matrix[i][j]){
                        /*AUMENTO L'OCCORRENZA ED AGGIORNO LE POSIZIONI*/
                        m.map[k1].row[m.map[k1].occorrenza] = i;
                        m.map[k1].column[m.map[k1].occorrenza] = j;
                        m.map[k1].occorrenza++;
                        //printf("carattere:%c, occorrenza:%d, row:%d, col:%d\n",m.matrix[i][j],m.map[k1].occorrenza,m.map[k1].row[m.map[k1].occorrenza],m.map[k1].column[m.map[k1].occorrenza]);
                        inserted = 1;
                        break;
                    }
                }
            }
            /*SE NON HO ANCORA TROVATO LA POSIZIONE VUOL DIRE CHE LA PROSSIMA SARà VUOTA*/
            if(inserted == 0){
                k++;
                m.map[k].carattere = m.matrix[i][j];
                m.map[k].row[m.map[k].occorrenza] = i;
                m.map[k].column[m.map[k].occorrenza] = j;
                m.map[k].occorrenza=1;
            }
        }
    }
    /*INVOCO LA ADJUST_CHARMAP PER RIDURRE L'OCCUPAZIONE DELLA MEMORIA*/
    m.map = Adjust_Charmap(m.map);
    /*INVOCO LA PRINT_CHARMAP PER STAMPARE LA MAPPATURA APPENA CREATA*/
    //Print_CharMap(m);
    return;
}

/*TROVA IL CARATTERE IN BASE ALLA MAPPATURA DELLA MATRICE*/
Charmap Find_Charmap_Element(Matrix m,char x){
    Charmap pos;
    for(int i = 0;i<m.size;i++){
        if (m.map[i].carattere == x){
            pos.row = (int*)malloc(m.map[i].occorrenza*sizeof(int));
            pos.column = (int*)malloc(m.map[i].occorrenza*sizeof(int));
            pos.carattere = m.map[i].carattere;
            for(int j = 0;j<m.map[i].occorrenza;j++){
                //printf("indice j:%d\n",j);
                pos.occorrenza = m.map[i].occorrenza;
                pos.row[j] = m.map[i].row[j];
                pos.column[j] = m.map[i].column[j];
                //printf("%c riga: %d,colonna: %d\n",pos.carattere,pos.row[j],pos.column[j]);
            }
            return pos;
        }
    }
    pos.occorrenza = 0; 
    return pos;
}



/*LIBERO LO SPAZIO NON NECESSARIO PRECEDENTEMENTE ALLOCATO*/
Charmap* Adjust_Charmap(Charmap* map){
    //int i = 0;
    for(int i =0;i<map->size;i++){
        if (map[i].carattere == '0'){
            map->size = i;
            break;
        }
    }
    /*DIMINUISCO LA MEMORIA ALLOCATA*/
    map = realloc(map, map->size *sizeof(Charmap));
    /*STAMPA PER DEBUGGING*/
    //Print_CharMap(newmap);   
    return map;
}

int Validate(Matrix m, char* word){
    /*DICHIARO ED INIZIALIZZO LA LISTA*/
    Position_List l = NULL;
    /*VENGONO ACCETTATE SOLO STRINGHE LUNGHE ALMENO 4 CARATTERI*/
    if (strlen(word)<4 ) return -1;
    /*COSTRUISCO UNA MAPPATURA DI TUTTE LE OCCORRENZE DEL PRIMO CARATTERE NELLA MATRICE*/
    Charmap start = Find_Charmap_Element(m,word[0]);
    /*PROVO A COSTRUIRE LA PAROLA SULLA MATRICE*/
    for(int i = 0;i<start.occorrenza;i++){
        /*MI ASSICURO CHE LA LISTA SIA VUOTA*/
        Delete_Position_List(&l);
        /*INSERISCO L'IESIMA OCCORRENZA DEL PRIMO CARATTERE*/
        Position_List_Push(&l,start.row[i],start.column[i]);
        /*PROVO A VALIDARE CON L'IESIMA OCCORRENZA*/
        Validation_Step(m,&l,word);
        /*SE */
        if (Position_List_Size(l)==strlen(word))return 0;
    }
    return -1;
}

void Validation_Step(Matrix m,Position_List* l,char * word){
    for(int j = 1;j<strlen(word);j++){
        int next_pos[2];
        /*MAPPO LE OCCORENZE DEL PROSSIMO CARATTERE*/
        Charmap found = Find_Charmap_Element(m,word[j]);
        /*CICLO SU TUTTE LE SUE OCCORRENZE*/
        for(int i=0;i<found.occorrenza;i++){
            /*MEMORIZZO LA POSIZONE IN UNA VARIABILE PER CHIAMARE LA FUNZIONE IS_REACHABLE*/
            next_pos[0] = found.row[i];
            next_pos[1] = found.column[i];
            /*controllo che la posizione del carattere sia raggiungibile e che la posizione non sia già presente in lista */
            if ((Is_Reachable(m,Position_List_Peek(*l),next_pos)==0 ) && (Position_List_Find(*l,next_pos[0],next_pos[1]) != 0)){
                /*AGGIUNGO IL NUOVO CARATTERE ALLA LISTA*/
                Position_List_Push(l,next_pos[0],next_pos[1]);
                break;
            }
        }
    } 
}



/*INSERISCO UN ELEMENTO IN TESTA ALLA LISTA*/
void Position_List_Push(Position_List* cl,int r, int c){
    Position_Node* el = (Position_Node*)malloc(sizeof(Position_Node));
    el->row = r;
    el->col = c;
    /*faccio puntare l'elemento alla testa della lista*/
    el->next = *cl;
    /*faccio puntare la testa della lista all'elemento*/
    *cl= el;
    return;
}

/*ESTRAGGO L'ELEMENTO IN TESTA ALLA LISTA*/
void Position_List_Pop(Position_List* pl){
    //int* popped = (int*)malloc(2*sizeof(int));
    // popped[0] = (*cl)->row;
    // popped[1] = (*cl)->col;
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

/*CERCO UN ELEMENTO ALL'INTERNO DELLA LISTA*/
int Position_List_Find(Position_List cl, int r, int c){
    if(cl == NULL)return -1;
    if(cl->row == r && cl->col == c)return 0;
    return Position_List_Find(cl->next,r,c);
}

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
    printf("r:%d,c:%d\n",cl->row,cl->col);
    return Print_Position_List(cl->next);
}

/*STAMPO IL VALORE DI RITORNO DI FIND_CHARMAP_ELEMENT*/
void Print_FCE(Charmap position,char carattere){
    for(int i= 0;i<position.occorrenza;i++){
        printf("carattere:%c,riga:%d,colonna:%d\n",carattere,position.row[i],position.column[i]); 
    }
    return;
}

/*STAMPA A SCHERMO LA MAPPATURA DEI CARATTERI DELLA MATRICE*/
void Print_CharMap(Charmap* map){
    int retvalue;
    char message[buff_size];
    for(int i =0;i<map->size;i++){
        sprintf(message,"%c:%d\n",map[i].carattere,map[i].occorrenza);
        writef(retvalue,message);
    }
    return;
}

/*STAMPA LA MATRICE*/
void Print_Matrix(Matrix m,char special, char exception){
    int retvalue;char message[buff_size];
    
    for(int i =0;i<m.rows;i++){
        for(int j = 0;j<m.columns;j++){
            /*SCRIVO IN UN BUFFER L'ELEMENTO DELLA MATRICE*/
            if (m.matrix[i][j] == special){
                sprintf(message,"%c%c\t", m.matrix[i][j],exception);
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