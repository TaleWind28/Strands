/*STRUTTURE DATI*/

/*STRUTTURA DATI CHE PERMETTE DI CERCARE PIù VELOCEMENTE TUTTE LE OCCORRENZE DI UN CARATTERE NELLA MATRICE*/
typedef struct{
    char carattere;
    int occorrenza;
    int* row;
    int* column;
    int size;
}Charmap;

/*STRUTTURA DATI CHE SEMPLIFICA IL LAVORO CON LE MATRICI*/
typedef struct{
    int rows;
    int columns;
    char** matrix;
    int size;
    Charmap* map;
}Matrix;

/*COMPOSE_NODE*/
typedef struct p{
    int row;
    int col;
    struct p* next;
}Position_Node;

typedef Position_Node* Position_List;

/*COSTRUZIONE ED INIZIALIZZAZIONE DI STRUTTURE DATI*/

/*CREA UNA MATRICE DATI IN INPUT IL NUMERO DI RIGHE E COLONNE E LA RITORNO*/
Matrix Create_Matrix(int rows, int columns);

/*INIZIALIZZA UNA MATRICE DI CARATTERI CON '0'*/
void zeros(Matrix m);

/*RIEMPIE UNA RIGA DELLA MATRICE*/
void Fill_Matrix_Row(Matrix m,int row,char* letter);

/*RIEMPIO TUTTA LA MATRICE IN BASE ALLA STRINGA PASSATA*/
void Fill_Matrix(Matrix m,char* letters);

/*CARICA IN UNA MATRICE UNA RIGA DI UN FILE*/
void Load_Matrix(Matrix m, char* path_to_file);

/*RIEMPIO LA MAPPATURA DEI CARATTERI DELLA MATRICE*/
void Build_Charmap(Matrix m);

/*LIBERO LA MEMORIA PRECEDENTEMENTE ALLOCATA E CHE NON VIENE USATA*/
Charmap* Adjust_Charmap(Charmap* map);


/*OPERAZIONI SULLE STRUTTURE DATI DEFINITE PRECEDENTEMENTE*/

/*RENDO LA MATRICE UNA STRINGA*/
void Stringify_Matrix(Matrix m,char* string);

/*CONTROLLO CHE IL NUMERO DI OCCORRENZE DEI CARATTERI SIA UGUALE A QUELLO DELLA MATRICE PER INIZIARE A VALIDARE AL PAROLA*/
int Is_Composable(Matrix m,char* word);

/*CONTROLLO SE UN ELEMENTO È RAGGIUNGIBILE*/
int Is_Reachable(Matrix m,int* old_pos,int* pos);

// /*CONTROLLA CHE UNA PAROLA SIA COMPONIBILE NELLA MATRICE DATA IN INPUT*/
int Validate_Word(Matrix m,Charmap* map, char* word);

/*TROVA UN ELEMENTO DELLA MATRICE IN BASE ALLA MAPPATURA*/
Charmap Find_Charmap_Element(Matrix m,char x);

/*CONTROLLO SE DUE ELEMENTI DELLA MATRICE SONO RAGGIUNGIBILI*/
Charmap Is_Charmap_Reachable(Charmap m1,Charmap m2,Charmap matches);


/*STAMPE A VIDEO DELLE STRUTTURE DATI O DELLE FUNZIONI SOPRA DEFINITE*/

/*STAMPA LA MATRICE*/
void Print_Matrix(Matrix m);

/*STAMPA A SCHERMO UNA MAPPATURA DI CARATTERI*/
void Print_CharMap(Charmap* m);

/*STAMPO IL VALORE DI RITORNO DI FIND_CHARMAP_ELEMENT*/
void Print_FCE(Charmap position,char carattere);


/*CONTROLLO SE HO UN MATCH SU RIGHE E COLONNE*/
int Is_Matching(int row, int col, Charmap map);

/*COMPOSABLE LIST*/

/*INSERISCO UN ELEMENTO IN TESTA ALLA LISTA*/
void Position_List_Push(Position_List* pl,int r, int c);

/*ESTRAGGO L'ELEMENTO IN TESTA ALLA LISTA*/
void Position_List_Pop(Position_List* pl);

/*CONTO GLI ELEMENTI DELLA LISTA*/
int Position_List_Size(Position_List pl);

/*COMUNICO LA TESTA DELLA LISTA*/
int* Position_List_Peek(Position_List pl);

/*CERCO UN ELEMENTO ALL'INTERNO DELLA LISTA*/
int Find_Compose(Position_List pl, int r, int c);

/*STAMPO LA LISTA*/
int Print_Position_List(Position_List pl);
/*nuova validate*/
int Validate(Matrix m, char* word);

/*distruggo una lista*/
int Delete_Position_List(Position_List* l);