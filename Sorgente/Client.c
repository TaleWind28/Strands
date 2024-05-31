//questa serve per non avere gli error squiggles sulla sigaction
#define _XOPEN_SOURCE 700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <pthread.h>
#include <errno.h>
#include "../Header/macro.h"
#include "../Header/Matrix.h"
#include "../Header/Queue.h"
#include "../Header/Communication.h"

#define NUM_ROWS 4
#define NUM_COLUMNS 4
#define RULES "Nel gioco del paroliere si indovinano parole dalla matrice e vengono assegnati dei punti in base alla lunghezza della parola.\nNella nostra versione si accettano solo parole di lunghezza superiore a 3 e vista la scarsità di parole con la Q nella matrice quest'ultima sarà sempre accompagnata da una U che conterà come lettera successiva, ad esempio la parola QUASI può essere composta in questo modo e vale 5 punti.\n"
#define WELCOME_MESSAGE "Ciao, benvenuto nel gioco del paroliere ,per prima cosa se non lo hai già fatto resgistrati mediante il comando registra_utente seguito dal tuo nome, per unirti ad una partita in corso o aspettare l'inizio di una nuova.\nDurante la partita potrai usare i seguenti comandi:\np <parola> \tper indovinare una parola presente nella matrice che vedi a schermo\n matrice\tper visualizzare a schermo la matrice ed il tempo residuo di gioco o di attesa\naiuto\tche ti mostra i comandi a te disponibili\nscore\tche ti mostra il tuo punteggio attuale\nfine\tche ti fa uscire dalla partita in corso\n"
#define HELP_MESSAGE "Puoi utilizzare i seguenti comandi:p <parola> \tper indovinare una parola presente nella matrice che vedi a schermo\nmatrice\tper visualizzare a schermo la matrice ed il tempo residuo di gioco o di attesa\naiuto\tche ti mostra i comandi a te disponibili\nscore\tche ti mostra il tuo punteggio attuale\nfine\tche ti fa uscire dalla partita in corso\n"
int client_fd;
void* bounce(void* args);
void* trade(void* args);
pthread_t bouncer,merchant;
Matrix matrice_player;

void gestione_terminazione_errata(int signum) {
    int retvalue;
    switch (signum){
        case SIGINT:
            printf("sigint\n");
            pthread_kill(merchant,SIGUSR2);
            pthread_cancel(bouncer);
            pthread_join(merchant,NULL);
            SYSC(retvalue,close(client_fd),"chiusura del client");
            /*chiudo il socket*/
            exit(EXIT_SUCCESS);
            return;
        
        case SIGUSR1:
            writef(retvalue,"Ci scusiamo per il disagio ma il server ha deciso di morire, grazie per aver giocato\n");
            //pthread_cancel(bouncer);
            pthread_exit(NULL);
            return;
        case SIGUSR2:
            writef(retvalue,"Grazie per aver giocato\n");
            if (pthread_self()==merchant){
                Send_Message(client_fd,"chiudo",MSG_CHIUSURA_CONNESSIONE);
            }
            pthread_exit(NULL);
            return;
    }
}

int main(int argc, char* argv[]){
    struct sigaction azione_segnale;
    sigset_t maschera_segnale;
    /*dichiarazione ed inizializzazione variabili*/
    int retvalue;
    struct sockaddr_in server_address;
    socklen_t server_len = sizeof(server_address);
    /*INIZIALIZZAZIONE VARIABILI IN BASE ALL'ARGOMENTO DELLA RIGA DI COMANDO*/
    char* HOST = argv[1];
    int PORT = atoi(argv[2]);
    //main_thread = pthread_self();
    /*CONTROLLO PARAMETRI RIGA DI COMANDO*/
    if(argc != 3){
        perror("usare la seguente sintassi: nome programa host porta_server");
        exit(EXIT_FAILURE);
    }
    /*inizializzazione server_address*/
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(HOST);
    server_address.sin_port = htons(PORT);
    
    /*creo il socket per comunicare*/
    SYSC(client_fd,socket(AF_INET,SOCK_STREAM,0),"nella creazione del client");
        
    /*chiamo la connect*/
    SYSC(retvalue,connect(client_fd,(struct sockaddr*)&server_address,server_len),"nella connect");
    writef(retvalue,"connesso\n");
    /*IMPOSTO LA MASCHERA*/
    sigemptyset(&maschera_segnale);
    SYSC(retvalue,sigaddset(&maschera_segnale,SIGINT),"aggiunta SIGINT alla maschera");
    SYSC(retvalue,sigaddset(&maschera_segnale,SIGUSR1),"aggiunta SIGINT alla maschera");
    SYSC(retvalue,sigaddset(&maschera_segnale,SIGUSR2),"aggiunta SIGINT alla maschera");

    /*IMPOSTO LA SIGACTION*/
    azione_segnale.sa_handler = gestione_terminazione_errata;
    azione_segnale.sa_mask = maschera_segnale;
  
    /*IMPOSTO IL GESTORE*/
    sigaction(SIGINT,&azione_segnale,NULL);
    sigaction(SIGUSR1,&azione_segnale,NULL);
    sigaction(SIGUSR2,&azione_segnale,NULL);

    matrice_player = Create_Matrix(4,4);
    writef(retvalue,WELCOME_MESSAGE);
    writef(retvalue,RULES);
    SYST(retvalue,pthread_create(&bouncer,NULL,bounce,&client_fd),"nella creazione del bouncer");
    SYST(retvalue,pthread_create(&merchant,NULL,trade,&client_fd),"nella creazione del mercante");
    //SYST(retvalue,pthread_detach(merchant),"nella detach");
    SYST(retvalue,pthread_join(bouncer,NULL),"attesa del bouncer");    
    SYST(retvalue,pthread_join(merchant,NULL),"attesa del mercante");
    /*chiudo il socket*/
}

void* bounce(void* args){
    int comm_fd = *(int*)args;
    char type = '0';
    int retvalue;
    //aspetto la risposta del server
    while (1){
        char* answer = Receive_Message(comm_fd,&type);
        switch(type){
            case MSG_MATRICE:
                //riempio la matrice
                Fill_Matrix(matrice_player,answer);
                //stampo la matrice al client
                writef(retvalue,"Questa è la matrice su cui giocare\n");
                Print_Matrix(matrice_player,'?','Q');
                break;
            
            case MSG_TEMPO_ATTESA:
                //stampo al client la durata residua
                //writef(retvalue,"Durata residua pausa ");
                writef(retvalue,answer);
                break;
            
            case MSG_TEMPO_PARTITA:
                writef(retvalue,"Durata residua partita ");
                writef(retvalue,answer);
                break;
            
            case MSG_PUNTI_PAROLA:
                writef(retvalue,answer);
                break;
            case MSG_PUNTEGGIO:
                writef(retvalue,answer);
                break;
            case MSG_PUNTI_FINALI:
                //scorer
                writef(retvalue,"classifica finale\n");
                writef(retvalue,answer);
                break;

            case MSG_ERR:
                writef(retvalue,answer);
                break;
            
            case MSG_OK:
                writef(retvalue,answer);
                break;

            case MSG_CHIUSURA_CONNESSIONE:
                writef(retvalue,"Chiusura client causa morte del server\n");
                //mando un SIGUSR1 al thread principale
                SYST(retvalue,pthread_kill(merchant,SIGUSR1),"nell'avvisare il mercante della chiusura");
                return NULL;
        }
        //free(answer);
    }
    
    return NULL;
}

void* trade(void* args){
    int comm_fd = *(int*) args;
    int retvalue;ssize_t n_read;
    char input_buffer[buff_size];
    while(1){
        SYSC(n_read,read(STDIN_FILENO,input_buffer,buff_size),"nella lettura da stdin");
        char* input = (char*)malloc(n_read+1);
        
        strncpy(input,input_buffer,n_read);
        input[n_read] = '\0';
        char* token = strtok(input," ");
        switch(input[0]){
            case 'a':
                writef(retvalue,HELP_MESSAGE);
                break;
            case 'r': 
                token = strtok(NULL,"\n");
                if (token == NULL || token[0] == '\n'){
                    writef(retvalue,"nome utente non valido\n");
                    break;
                }
                //writef(retvalue,token)
                //invio al server il messaggio con le credenziali per la registrazione
                Send_Message(comm_fd,token,MSG_REGISTRA_UTENTE);
                break;
            case 'm':
                //invio al server la richiesta della matrice
                Send_Message(comm_fd,"matrice",MSG_MATRICE);
                break;
            case 'p':
                //tokenizzo la stringa per ottenere la parla inserita dall'utente
                token = strtok(NULL,"\n");
                //controllo che il token contenga qualcosa 
                if (token == NULL || token[0] == '\n'){
                    //se il token è vuoto la parola è automaticamente sbagliata, quindi la considero come parola lunga 3 minuscola con il \n
                    Send_Message(comm_fd,"lol\n",MSG_PAROLA);
                }else{
                    //rendo maiuscola la stringa
                    Caps_Lock(token);
                    //invio al server la parola
                    Send_Message(comm_fd,token,MSG_PAROLA);
                }
                break;
            case 'f':
                //comunico al server che l'utente si sta disconnettendo
                Send_Message(comm_fd,"fine",MSG_CHIUSURA_CONNESSIONE);
                SYST(retvalue,pthread_kill(bouncer,SIGUSR2),"nell'avviso verso il bouncer della chiusura");
                SYSC(retvalue,close(client_fd),"nella chiusura del client per scelta utente");
                return NULL;
            case 'c':
                Send_Message(comm_fd,"classifica",MSG_PUNTI_FINALI);
                break;
            default:
                if (strcmp(input,"score\n")==0){
                    Send_Message(comm_fd,"punti",MSG_PUNTEGGIO);
                    break;
                }
                //stampa di default
                writef(retvalue,"comando non disponibile, digitare aiuto per una lista dettagliata\n");
                break;
        }
        //free(input);
    }  
    return NULL;
}