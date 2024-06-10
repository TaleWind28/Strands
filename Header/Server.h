#define MAX_NUM_CLIENTS 32
#define NUM_ROWS 4
#define NUM_COLUMNS 4
#define DIZIONARIO "../Text/Dizionario.txt"
#define MATRICI "../Text/Matrici.txt"
#define DURATA_PAUSA  60//60 secondi
/*usata per debugging*/
//#define DURATA_PARTITA 5//60 secondi
#include <stdio.h>
#include <unistd.h>
#include <string.h>

typedef struct {
    char* matrix_file;
    int durata_partita;
    long seed;
    char* file_dizionario;
}Parametri;

enum {
    OPT_MATRICI = 1,
    OPT_DURATA,
    OPT_SEED,
    OPT_DIZ,
};

typedef struct {
    char* username;
    int points;
}giocatore;

/*THREAD FUNCTIONS*/
void* Thread_Handler(void* args);
void* Gestione_Server(void* args);
void* scoring(void* args);

/*GENERAL FUNCTIONS*/
void Init_Params(int argc, char*argv[],Parametri* params);
void Init_SIGMASK();
void Choose_Action(int client_fd,char type,char* input,Word_List* already_guessed,int* points);
void Generate_Round();
void Load_Dictionary(Trie* Dictionary, char* path_to_dict);
void Replace_Special(char* string,char special);
int Check_Words(char* string,off_t* offset, int file_descriptor);
char* tempo(int max_dur);

/*GLOBAL VARIABLES*/
Parametri parametri_server;
Word_List Scoring_List;
Player_List Players;
char* matrice_di_gioco;
Trie* Dizionario;
pthread_mutex_t player_mutex,scorer_mutex,client_mutex;

List Client_List;
Graph* matrice_grafo;

char* HOST;
int PORT;
int game_on = 0,ready = 0,game_starting,client_attivi = 0,score_time = 0; 
char classifica[2048];
int server_fd,client_fd;
time_t start_time,end_time;
pthread_t jester,scorer,main_tid;