#include <pthread.h>

/*WORD_NODE*/
typedef struct w{
    char* word;
    pthread_t handler;
    int points;
    int client_fd;
    struct w* next;
}Word_Node;

/*WORD_LIST*/
typedef Word_Node * Word_List;

/*INSERISCO UN ELEMENTO IN TESTA ALLA LISTA*/
void WL_Push(Word_List* wl,char* word);

/*ESTRAGGO L'ELEMENTO IN TESTA ALLA LISTA*/
char* WL_Pop(Word_List* wl);
//ripensare
//prendi il punteggio
int WL_Retrieve_Score(Word_List wl,pthread_t gestore);

int WL_Update_Score(Word_List wl,pthread_t gestore,int new_score);

//placeholder per la nuova struttura
void WL_Push_Thread(Word_List* wl,char* word, int socket_fd);

//prendi il client_fd
int WL_Retrieve_Socket(Word_List wl,pthread_t gestore);

/*CONTO GLI ELEMENTI DELLA LISTA*/
int WL_Size(Word_List wl);

/*COMUNICO LA TESTA DELLA LISTA*/
char* WL_Peek(Word_List wl);

/*COMUNICO IL GESTORE DELLA TESTA DELLA LISTA*/
pthread_t WL_Peek_Hanlder(Word_List wl);

/*CERCO UN ELEMENTO ALL'INTERNO DELLA LISTA*/
int WL_Find_Word(Word_List wl, char* word);

int WL_Splice(Word_List* wl,char* word);

/*STAMPO LA LISTA*/
int Print_WList(Word_List wl);

char* WL_Retrieve_User(Word_List wl,pthread_t gestore);


