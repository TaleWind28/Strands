/*STRUTTURE DATI*/

typedef struct{
    // Numero di nodi nel grafo
    int V; 
    //Array di caratteri per le etichette dei nodi
    char *nodes;
    //Lista di adiacenza
    int **adjList; 
}Graph;

typedef enum {false,true} bool;

Graph* Build_Graph(char* matrix,int ROWS, int COLS);
/*COSTRUZIONE ED INIZIALIZZAZIONE DI STRUTTURE DATI*/
Graph* createGraph(int V);

void addEdge(Graph* graph, int src, int dest);

bool dfsUtil(Graph* graph, int current, char* word, int index, bool* visited);

bool dfs(Graph* graph, char* word); 

/*CARICA IN UNA MATRICE UNA RIGA DI UN FILE*/
char* Load_Matrix(char* path_to_file,char exception,int* offset);

char* File_Read(int fd, char exception, int*offset);
/*OPERAZIONI SULLE STRUTTURE DATI DEFINITE PRECEDENTEMENTE*/

/*CONTROLLO SE UN ELEMENTO È RAGGIUNGIBILE*/
int Is_Reachable(int* old_pos,int* pos);

/*RIMOVERE UN CARATTERE DA UNA STRINGA*/
void Adjust_String(char* string,char x);

/*STAMPE A VIDEO DELLE STRUTTURE DATI O DELLE FUNZIONI SOPRA DEFINITE*/

/*STAMPA LA MATRICE*/
void Print_Matrix(char* m,int NUM_ROWS,int NUM_COLUMNS,char special);
