/*WORD_NODE*/
typedef struct w{
    char* word;
    struct w* next;
}Word_Node;

typedef Word_Node * Word_List;

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

