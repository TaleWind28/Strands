#define NUM_CHAR 26

typedef struct t{
    struct t* figli[NUM_CHAR];
    int is_word;
}Trie;
//creo un nodo dell'albero
Trie* create_node();
//inserisco una parola all'interno dell'albero
int insert_Trie(Trie* trie, char* word);
//cerco una parola all'interno dell'albero
int search_Trie(char* word, Trie* trie);
//stampo tutte le parole dell'albero
void Print_Trie(Trie* trie, char* buffer,int depth);