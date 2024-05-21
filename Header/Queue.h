/*WORD_NODE*/
typedef struct w{
    char* word;
    struct w* next;
}Word_Node;

/*WORD_LIST*/
typedef Word_Node * Word_List;

/*INSERISCO UN ELEMENTO IN TESTA ALLA LISTA*/
void WL_Push(Word_List* wl,char* word);

/*ESTRAGGO L'ELEMENTO IN TESTA ALLA LISTA*/
char* WL_Pop(Word_List* wl);

/*CONTO GLI ELEMENTI DELLA LISTA*/
int WL_Size(Word_List wl);

/*COMUNICO LA TESTA DELLA LISTA*/
char* WL_Peek(Word_List wl);

/*CERCO UN ELEMENTO ALL'INTERNO DELLA LISTA*/
int WL_Find_Word(Word_List wl, char* word);

int WL_Splice(Word_List* wl,char* word);

/*STAMPO LA LISTA*/
int Print_WList(Word_List wl);


