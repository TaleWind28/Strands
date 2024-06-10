#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/wait.h>
#include "../Header/macro.h"
#include "../Header/Matrix.h"

#define ROWS 4
#define COLS 4

// Definizione del tipo booleano
typedef enum { false, true } bool;
// // Definizione della struttura del grafo
// struct Graph {
//     int V; // Numero di nodi nel grafo
//     int **adjList; // Lista di adiacenza
// };

// // Funzione per creare un nuovo grafo
// struct Graph* createGraph(int V) {
//     struct Graph* graph = (struct Graph*) malloc(sizeof(struct Graph));
//     graph->V = V;

//     // Allocazione della memoria per la lista di adiacenza
//     graph->adjList = (int**) malloc(V * sizeof(int*));
//     for (int i = 0; i < V; i++) {
//         graph->adjList[i] = (int*) malloc(V * sizeof(int));
//     }

//     return graph;
// }

// Matrice di caratteri rappresentata come una stringa
char* matrix = "ATLCIOQADVESISBI";

//Definizione della struttura del grafo
struct Graph {
    int V; // Numero di nodi nel grafo
    char *nodes; // Array di caratteri per le etichette dei nodi
    int **adjList; // Lista di adiacenza
};

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

int main(int argc,char* argv[]){
    // if(argc!=6){
    //     perror("usare la seguente sintassi: nome_programma rows columns contenuto_matrice word");
    //     exit(EXIT_FAILURE);
    // }
    // int rows = atoi(argv[1]);
    // int columns = atoi(argv[2]);
    // //char* input = argv[3];
    // char* word = argv[4];
    // char* m_string = argv[5];
    // Matrix matrix = Create_Matrix(rows,columns);
    // Load_Matrix(matrix,m_string,'U',0);
    // //Adjust_String(input,'u');
    // //Fill_Matrix(matrix,input);
    // Print_Matrix(matrix,'Q','U');
    
    // char* string = (char*)malloc(matrix.size*(sizeof(char)));
    // string = Stringify_Matrix(matrix);
    // char message[buff_size];
    // int retvalue;
    // sprintf(message,"stringa matriciale indotta: %s\n",string);
    // writef(retvalue,message);
    // //Fill_Matrix(matrix,string);
    // Build_Charmap(matrix);
    // Creazione del grafo
 // Creazione del grafo
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

    // Stampa il grafo
    printGraph(graph);
    // Test della DFS per parole specifiche
    char* word1 = "ABEF"; // Parola valida
    char* word2 = "QESTO"; // Parola non valida
    char* word = (char*)malloc(256);
    
    while(1){
        read(0,word,256);
        word = strtok(word,"\n");
        printf("%s: %s\n", word, dfs(graph, word) ? "Valida" : "Non valida");
        //free(word);
    }
    return 0;
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
    // Adjust_String(word,'u');
    // printf("nuova validate:%d\n",Validate(matrix,word));
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

    //exit(0);
}