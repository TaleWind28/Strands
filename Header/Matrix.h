/*STRUTTURE DATI*/

struct Graph {
    int V; // Numero di nodi nel grafo
    char *nodes; // Array di caratteri per le etichette dei nodi
    int **adjList; // Lista di adiacenza
};

typedef enum {false,true} bool;

struct Graph* Build_Graph(char* matrix,int ROWS, int COLS);
/*COSTRUZIONE ED INIZIALIZZAZIONE DI STRUTTURE DATI*/
struct Graph* createGraph(int V);

void addEdge(struct Graph* graph, int src, int dest);

void printGraph(struct Graph* graph);

bool dfsUtil(struct Graph* graph, int current, char* word, int index, bool* visited);

bool dfs(struct Graph* graph, char* word); 

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
