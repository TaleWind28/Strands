/*STRUTTURE DATI*/

/*STRUTTURA DATI CHE SEMPLIFICA IL LAVORO CON LE MATRICI*/
typedef struct{
    int rows;
    int columns;
    char** matrix;
    int size;
}Matrix;

/*NODO DI UNA LISTA CHE MEMORIZZA LE POSIZIONI DELLA MATRICE*/
typedef struct p{
    int row;
    int col;
    int used;
    char val;
    struct p* next;
}Position_Node;

struct Graph {
    int V; // Numero di nodi nel grafo
    char *nodes; // Array di caratteri per le etichette dei nodi
    int **adjList; // Lista di adiacenza
};


typedef enum {false,true} bool;

typedef Position_Node* Position_List;

struct Graph* Build_Graph(char* matrix,int ROWS, int COLS);
/*COSTRUZIONE ED INIZIALIZZAZIONE DI STRUTTURE DATI*/
struct Graph* createGraph(int V);
void addEdge(struct Graph* graph, int src, int dest);
void printGraph(struct Graph* graph);
bool dfsUtil(struct Graph* graph, int current, char* word, int index, bool* visited);
bool dfs(struct Graph* graph, char* word); 
/*CREA UNA MATRICE DATI IN INPUT IL NUMERO DI RIGHE E COLONNE E LA RITORNO*/
Matrix Create_Matrix(int rows, int columns);

/*INIZIALIZZA UNA MATRICE DI CARATTERI CON '0'*/
void zeros(Matrix m);
int Is_Composable(Position_List pl, char* string);
int* Find_Pos(Position_List pl, char x,int skip);
int Find_Max_Occ(Position_List pl, char x);
/*RIEMPIE UNA RIGA DELLA MATRICE*/
void Fill_Matrix_Row(Matrix m,int row,char* letter);

/*RIEMPIO TUTTA LA MATRICE IN BASE ALLA STRINGA PASSATA*/
void Fill_Matrix(Matrix m,char* letters);

/*CARICA IN UNA MATRICE UNA RIGA DI UN FILE*/
void Load_Matrix(Matrix m, char* path_to_file,char exception,int* offset);

char* File_Read(int fd, char exception, int*offset);




/*OPERAZIONI SULLE STRUTTURE DATI DEFINITE PRECEDENTEMENTE*/

/*RENDO LA MATRICE UNA STRINGA*/
char* Stringify_Matrix(Matrix m);

/*CONTROLLO SE UN ELEMENTO È RAGGIUNGIBILE*/
int Is_Reachable(int* old_pos,int* pos);

/*RIMOVERE UN CARATTERE DA UNA STRINGA*/
void Adjust_String(char* string,char x);

/*COMPOSABLE LIST*/
Position_List Position_List_Build(char* string,int NUM_ROW,int NUM_COL);

/*INSERISCO UN ELEMENTO IN TESTA ALLA LISTA*/
void Position_List_Push(Position_List* pl,char x,int r, int c);

/*ESTRAGGO L'ELEMENTO IN TESTA ALLA LISTA*/
void Position_List_Pop(Position_List* pl);

/*CONTO GLI ELEMENTI DELLA LISTA*/
int Position_List_Size(Position_List pl);

/*COMUNICO LA TESTA DELLA LISTA*/
int* Position_List_Peek(Position_List pl);

/*CERCO UN ELEMENTO ALL'INTERNO DELLA LISTA*/
int Position_List_Find(Position_List cl, int r, int c);

/*STAMPO LA LISTA*/
int Print_Position_List(Position_List pl);

/*DISTRUGGO UNA LISTA*/
int Delete_Position_List(Position_List* l);

/*STAMPE A VIDEO DELLE STRUTTURE DATI O DELLE FUNZIONI SOPRA DEFINITE*/

/*STAMPA LA MATRICE*/
void Print_Matrix(Matrix m,char special, char exception);
