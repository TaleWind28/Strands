#define NUM_CHAR 256

typedef struct Trie{
    struct Trie* figli[NUM_CHAR];
    int is_word;
}Trie;

Trie* create_node();
int search_Trie(char* word, Trie* trie);
int insert_Trie(Trie* trie, char* word);
void Print_Trie(Trie* trie, char* buffer,int depth);