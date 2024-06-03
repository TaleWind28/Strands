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


/*CARICO NELLA MATRICE IL CONTENUTO DI UNA RIGA DI UN FILE*/
char* Load_Matrix(char* path_to_file,char exception,int* offset){
    int fd,retvalue;char* buffer;
    
    /*APRO IL FILE IN BASE AL PATH PASSATO*/
    SYSC(fd,open(path_to_file,O_RDONLY),"nell'apertura del file");
    
    /*LEGGO LA PRIMA RIGA DEL FILE*/
    buffer = File_Read(fd,exception,offset);
    Caps_Lock(buffer);
    /*AGGIORNO LA MATRICE IN BASE ALLA LETTURA*/
    
    //writef(retvalue,buffer);
    Adjust_String(buffer,exception);
    //Fill_Matrix(m,buffer);
    /*CHIUDO IL FILE*/
    SYSC(retvalue,close(fd),"nella chiusura del file descriptor");
    return buffer;
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

/*STAMPA LA MATRICE*/
void Print_Matrix(char* m,int rows, int columns, char special){
    int retvalue;char message[buff_size];
    for(int i =0;i<rows;i++){
        for(int j = 0;j<columns;j++){     
            /*SCRIVO IN UN BUFFER L'ELEMENTO DELLA MATRICE*/
            if (m[i] == special){
                sprintf(message,"%c%c\t",'Q','U');
            }
            else{
                sprintf(message,"%c\t", m[i*columns+j]);
            }
            /*SCRIVO A VIDEO L'ELEMENTO*/
            writef(retvalue,message);
        }
        writef(retvalue,"\n");
    }

    return;
}

// Funzione per creare un nuovo grafo
struct Graph* createGraph(int V) {
    struct Graph* graph = (struct Graph*) malloc(sizeof(struct Graph));
    graph->V = V;
    graph->nodes = (char*) malloc(V * sizeof(char));

    // Allocazione della memoria per la lista di adiacenza
    graph->adjList = (int**) malloc(V * sizeof(int*));
    for (int i = 0; i < V; i++) {
        graph->adjList[i] = (int*) malloc(V * sizeof(int));
    }

    return graph;
}

// Funzione per aggiungere un arco al grafo
void addEdge(struct Graph* graph, int src, int dest) {
    graph->adjList[src][dest] = 1;
}

// Funzione per stampare il grafo
void printGraph(struct Graph* graph) {
    printf("Grafo:\n");
    for (int i = 0; i < graph->V; i++) {
        printf("%c -> ", graph->nodes[i]);
        for (int j = 0; j < graph->V; j++) {
            if (graph->adjList[i][j] == 1) {
                printf("%c ", graph->nodes[j]);
            }
        }
        printf("\n");
    }
}

// Funzione di supporto per la DFS
bool dfsUtil(struct Graph* graph, int current, char* word, int index, bool* visited) {
    // Se abbiamo raggiunto la fine della parola, la parola è valida
    if (word[index] == '\0')
        return true;

    // Segna il nodo corrente come visitato
    visited[current] = true;

    // Cerca tutti i nodi adiacenti non visitati
    for (int i = 0; i < graph->V; i++) {
        if (graph->adjList[current][i] == 1 && !visited[i] && graph->nodes[i] == word[index]) {
            // Se troviamo una corrispondenza e il nodo non è stato visitato, continuiamo la ricerca
            if (dfsUtil(graph, i, word, index + 1, visited))
                return true;
        }
    }

    // Se non c'è corrispondenza trovata o non esistono nodi adiacenti non visitati,
    // marchiamo il nodo corrente come non visitato e restituiamo falso
    visited[current] = false;
    return false;
}

// Funzione per eseguire la DFS per validare una parola sul grafo
bool dfs(struct Graph* graph, char* word) {
    // Inizializza un array di booleani per tenere traccia dei nodi visitati
    bool* visited = (bool*) malloc(graph->V * sizeof(bool));
    memset(visited, false, graph->V * sizeof(bool));

    // Ciclo sui nodi del grafo
    for (int i = 0; i < graph->V; i++) {
        // Se il nodo corrente corrisponde al primo carattere della parola e la DFS restituisce true, la parola è valida
        if (graph->nodes[i] == word[0] && dfsUtil(graph, i, word, 1, visited))
            return true;
    }

    // Se non viene trovata una corrispondenza per il primo carattere della parola, o se la DFS restituisce false, la parola non è valida
    return false;
}

struct Graph* Build_Graph(char* matrix,int ROWS, int COLS){
    int V = strlen(matrix);
    struct Graph* graph = createGraph(V);

    // Associazione dei nodi con i caratteri della matrice
    for (int i = 0; i < V; i++) {
        graph->nodes[i] = matrix[i];
    }

    // Generazione dei collegamenti basati sulla matrice
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            int node = i * COLS + j;

            // Aggiungi archi solo se possibile (orizzontale e verticale)
            if (i > 0) addEdge(graph, node, (i - 1) * COLS + j); // nodo sopra
            if (i < ROWS - 1) addEdge(graph, node, (i + 1) * COLS + j); // nodo sotto
            if (j > 0) addEdge(graph, node, i * COLS + (j - 1)); // nodo a sinistra
            if (j < COLS - 1) addEdge(graph, node, i * COLS + (j + 1)); // nodo a destra
        }
    }
    return graph;
}