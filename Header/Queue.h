/*WORD_NODE*/
typedef struct w{
    char* word;
    struct w* next;
}Word_Node;

typedef Word_Node * Word_List;

#define TABLE_SIZE 524288 // Dimensione della hash table, scelta per essere il doppio di 280k

// Struttura per una voce nella hash table
typedef struct {
    char *string;
    int is_occupied;
} Hash_Entry;

/*WORD_LIST*/

/*INSERISCO UN ELEMENTO IN TESTA ALLA LISTA*/
void Push(Word_List* wl,char* word);

/*ESTRAGGO L'ELEMENTO IN TESTA ALLA LISTA*/
char* Pop(Word_List* wl);

/*CONTO GLI ELEMENTI DELLA LISTA*/
int Size(Word_List wl);

/*COMUNICO LA TESTA DELLA LISTA*/
char* Peek(Word_List wl);

/*CERCO UN ELEMENTO ALL'INTERNO DELLA LISTA*/
int Find_Word(Word_List wl, char* word);

/*STAMPO LA LISTA*/
int Print_WList(Word_List wl);

// Funzione hash semplice (djb2)
unsigned int hash(const char *str);

// Funzione per inizializzare la hash table
void init_table(Hash_Entry *table);

// Funzione per inserire una stringa nella hash table usando linear probing
void insert_string(Hash_Entry *table, const char *str);

// Funzione per cercare una stringa nella hash table
int search_string(Hash_Entry *table, const char *str);

void Delete_Table(Hash_Entry* hash_table);

