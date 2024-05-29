#include <pthread.h>

/*WORD_NODE*/
typedef struct w{
    char* word;
    struct w* next;
}Word_Node;

/*NODE*/
typedef struct n{
    int val;
    pthread_t thread;
    struct n* next;
}Node;

typedef Node* List;

typedef struct pl{
    char* word;
    pthread_t handler;
    int points;
    int client_fd;
    struct pl* next;
}Player_Node;

/*WORD_LIST*/
typedef Word_Node * Word_List;

typedef Player_Node* Player_List;



//placeholder per la nuova struttura
void Player_Push_Thread(Player_List* wl,char* word, int socket_fd);

//rimuovo il primo elemento dalla lista
char* Player_Pop(Player_List* wl);

//prendo l'username
char* Player_Retrieve_User(Player_List wl,pthread_t gestore);
//prendi il punteggio
int Player_Retrieve_Score(Player_List wl,pthread_t gestore);
//aggiorno il punteggio
int Player_Update_Score(Player_List wl,pthread_t gestore,int new_score);
//prendi il client_fd
int Player_Retrieve_Socket(Player_List wl,pthread_t gestore);
/*CONTO GLI ELEMENTI DELLA LISTA*/
int Player_Size(Player_List wl);
/*COMUNICO IL GESTORE DELLA TESTA DELLA LISTA*/
pthread_t Player_Peek_Hanlder(Player_List wl);


/*INSERISCO UN ELEMENTO IN TESTA ALLA LISTA*/
void WL_Push(Word_List* wl,char* word);

/*CONTO GLI ELEMENTI DELLA LISTA*/
int WL_Size(Word_List wl);

/*COMUNICO LA TESTA DELLA LISTA*/
char* WL_Peek(Word_List wl);

/*ESTRAGGO L'ELEMENTO IN TESTA ALLA LISTA*/
char* WL_Pop(Word_List* wl);

/*CERCO UN ELEMENTO ALL'INTERNO DELLA LISTA*/
int WL_Find_Word(Word_List wl, char* word);


int Player_Splice(Player_List* wl,char* word);

/*STAMPO LA LISTA*/
int Print_WList(Word_List wl);

int Player_Find_Word(Player_List wl,char* word);


int L_Peek(List wl);

int L_Size(List wl);

int L_Pop(List* wl);

void L_Push(List* wl,int x);

int L_Splice(List* wl);