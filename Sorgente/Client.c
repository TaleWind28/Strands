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
#include <termios.h>
#include "../Header/macro.h"
#include "../Header/Matrix.h"
#include "../Header/Stack.h"
#include "../Header/Communication.h"

#define NUM_ROWS 4
#define NUM_COLUMNS 4
int client_fd;
void* bounce(void* args);
void* trade(void* args);
pthread_t bouncer,merchant,main_tid;
int unwanted_termination = 0;
char* matrice;
int only_space_string(char* string);
char type_sent;
pthread_cond_t awaiting_answer;
pthread_mutex_t user_mutex;

//GESTORE SEGNALI LATO CLIENT
void gestione_terminazione_errata(int signum) {
    int retvalue;
    switch (signum){
        //HO RICEVUTO UN SEGNALE DI TERMINAZIONE
        case SIGINT:
            //SE IL SERVER STA GIà TERMINANDO LO IGNORO  
            if (unwanted_termination == 1)return;
            //SE SONO IL MAIN THREAD LO GESTISCO
            if(pthread_self()== main_tid){
                //STAMPA DI DEBUG
                printf("sigint\n");
                //INVIO UN SEGNALE AL MERCHANT PER DIRGLI DI PREDISPOSRSI ALLA CHIUSURA
                SYST(retvalue,pthread_kill(merchant,SIGUSR2),"avviso il merchant della chiusura");
                //CANCELLO IL THREAD BOUNCER PERCHÈ NON STO ASPETTANDO NIENTE DAL SERVER
                SYST(retvalue,pthread_cancel(bouncer),"ammazzo il bouncer");
                //pthread_join(bouncer,NULL);
                //ASPETTO CHE IL MERCHANT ABBIA FINITO DI GESTIRE IL SEGNALE
                SYST(retvalue,pthread_join(merchant,NULL),"aspetto il merchant");
                //CHIUDO IL SOCKET DI COMUNICAZIONE
                SYSC(retvalue,close(client_fd),"chiusura del client");
                //TERMINO L'ESECUZIONE
                exit(EXIT_SUCCESS);
            }
            //pthread_exit(NULL);
            //SE NON SONO IL MAIN THREAD CONTINUO IL MIO FLUSSO D'ESECUZIONE
            else return;
        //HO RICEVUTO UN SEGNALE DI TERMINAZIONE
        //QUESTA GESTIONE È IDENTICA A QUELLA DEL SIGINT MA È STATA MESSA PER INDICARE CHE PER OGNI POSSIBILE SEGNALE CHE FA TERMINARE IL CLIENT È NECESSARIO FARLO CHIUDERE IN UNA CERTA
        //MANIERA PER NON INCAPPARE NELL'ERRORE BROKEN PIPE LATO SERVER CHE CAUSA LA MORTE DEL SERVER 
        //IL CODICE SEGUENTE NON VERRà COMMENTATO IN QUANTO È LA COPIA DI QUELLO DELLA GESTIONE SIGINT
        case SIGQUIT:
            if(pthread_self()== main_tid){
                printf("sigint\n");
                pthread_kill(merchant,SIGUSR2);
                pthread_cancel(bouncer);
                pthread_join(merchant,NULL);
                SYSC(retvalue,close(client_fd),"chiusura del client");
                /*chiudo il socket*/
                exit(EXIT_SUCCESS);
            }
            else return;
        //È ARRIVATO UN SEGNALE SIGUSR1, QUESTO SEGNALE VIENE MANDATO AL BOUNCER DAL MERCHANT PER TERMINARE L'ESECUZIONE UNA VOLTA RICEVUTO IL COMANDO FINE
        case SIGUSR1:
            pthread_exit(NULL);
            return;
        //È ARRIVATO UN SEGNALE SIGUSR2, QUESTO SEGNALE VIENE MANDATO AL MERCHANT DAL MAIN THREAD PER COMUNICARE DI CHIUDERE LA CONNESSIONE
        case SIGUSR2:
            //STAMPO ALL'UTENTE UN MESSAGGIO
            writef(retvalue,"Grazie per aver giocato\n");
            if (pthread_self()==merchant){
                //COMUNICO AL SERVER CHE STO TERMINANDO IL CLIENT
                Send_Message(client_fd,"chiudo",MSG_CHIUSURA_CONNESSIONE);
            }
            //TERMINO L'ESECUZIONE
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
        SYSC(retvalue,sigaddset(&maschera_segnale,SIGQUIT),"aggiunta SIGQUIT alla maschera");
    SYSC(retvalue,sigaddset(&maschera_segnale,SIGUSR1),"aggiunta SIGUSR1 alla maschera");
    SYSC(retvalue,sigaddset(&maschera_segnale,SIGUSR2),"aggiunta SIGUSR2 alla maschera");

    /*IMPOSTO LA SIGACTION*/
    azione_segnale.sa_handler = gestione_terminazione_errata;
    azione_segnale.sa_mask = maschera_segnale;
  
    /*IMPOSTO IL GESTORE*/
    sigaction(SIGINT,&azione_segnale,NULL);
    sigaction(SIGUSR1,&azione_segnale,NULL);
    sigaction(SIGUSR2,&azione_segnale,NULL);
    sigaction(SIGQUIT,&azione_segnale,NULL);

    pthread_cond_init(&awaiting_answer,NULL);
    pthread_mutex_init(&user_mutex,NULL);
    
    main_tid = pthread_self();
    //CREO I THREAD PER COMUNICARE COL SERVER
    SYST(retvalue,pthread_create(&bouncer,NULL,bounce,&client_fd),"nella creazione del bouncer");
    SYST(retvalue,pthread_create(&merchant,NULL,trade,&client_fd),"nella creazione del mercante");
    //ASPETTO LA TERMINAZIONE DEI THREAD
    SYST(retvalue,pthread_join(bouncer,NULL),"attesa del bouncer");    
    SYST(retvalue,pthread_join(merchant,NULL),"attesa del mercante");
    /*chiudo il socket*/
}

//THREAD CHE RICEVE MESSAGGI DAL SERVER 
void* bounce(void* args){
    int comm_fd = *(int*)args;
    char type = '0';
    int retvalue;
    //ATTENDO UNA RISPOSTA DAL SERVER
    while (1){
        char* answer = Receive_Message(comm_fd,&type);
        switch(type){
            case MSG_MATRICE:
                //stampo la matrice al client
                writef(retvalue,"Questa è la matrice su cui giocare\n");
                Print_Matrix(answer,4,4,'?');
                break;
            
            case MSG_TEMPO_ATTESA:
                //stampo al client la durata residua
                writef(retvalue,"Tempo mancante alla prossima partita:");
                writef(retvalue,answer);
                if (type_sent == MSG_MATRICE){
                    /*per testare la mutua esclusione sullo stdin*/
                    //sleep(2);
                    //writef(retvalue,"prima di unlockare\n");
                    pthread_mutex_unlock(&user_mutex);
                }
                break;
            
            case MSG_TEMPO_PARTITA:
                //STAMPO AL CLIENT LA DURATA RESIDUA DELLA PARTITA
                writef(retvalue,"Tempo restante per la partita in corso:");
                writef(retvalue,answer);
                if (type_sent == MSG_MATRICE){
                    //writef(retvalue,"prima di unlockare\n");
                    /*per testare la mutua esclusione prima di unlockare*/
                    //sleep(2);
                    pthread_mutex_unlock(&user_mutex);
                }
                break;
            
            case MSG_PUNTI_PAROLA:
                //STAMPO I PUNTI AL CLIENT
                writef(retvalue,answer);
                if (type_sent == MSG_PAROLA){
                    pthread_mutex_unlock(&user_mutex);
                }
                break;
            case MSG_PUNTEGGIO:
                //STAMPO IL PUNTEGGIO AL CLIENT
                writef(retvalue,answer);
                if (type_sent == MSG_PUNTEGGIO){
                    pthread_mutex_unlock(&user_mutex);
                }
                break;
            case MSG_PUNTI_FINALI:
                //scorer
                //SE LA CLASSIFICA NON È STATA STILATA COMUNICO CHE NON È DISPONIBILE
                if (strcmp("stringa vuota",answer)==0){
                    writef(retvalue,"Classifica non ancora disponibile\n");
                }else{
                    //ALTRIMENTI STAMPO LA CLASSIFICA COMPLETA ALL'UTENTE
                    writef(retvalue,"classifica finale\n");
                    char* token = strtok(answer,",");
                    while(token!=NULL){
                        writef(retvalue,token);
                        token = strtok(NULL,",");
                    }
                }
                pthread_mutex_unlock(&user_mutex); 
                break;

            case MSG_ERR:
                writef(retvalue,answer);
                pthread_mutex_unlock(&user_mutex);
                break;
            
            case MSG_OK:
                writef(retvalue,answer);
                pthread_mutex_unlock(&user_mutex);
                break;
            case MSG_CHIUSURA_CONNESSIONE:
                //STAMPO IL MESSAGGIO DEL SERVER ALL'UTENTE
                writef(retvalue,answer);
                //SE STO GIà TERMINANDO A CAUSA DEL SERVER LO IGNORO
                if (unwanted_termination == 1)return NULL;
                //mando un SIGUSR1 al MERCHANT
                SYST(retvalue,pthread_kill(merchant,SIGUSR1),"nell'avvisare il mercante della chiusura");
                return NULL;
            case MSG_SIGINT:
                //AVVISO L'UTENTE DEI PROBLEMI DEL SERVER
                writef(retvalue,answer);
                writef(retvalue,"non sarà possibile digitare altri comandi, ci scusiamo per il disagio\n");
                //MEMORIZZO LO STATO DI TERMINAZIONE OBBLIGATA DAL SERVER 
                unwanted_termination = 1;
                SYST(retvalue,pthread_kill(merchant,SIGUSR1),"nell'avvisare il mercante della chiusura");
                //return NULL;
        }
        //free(answer);
        //STAMPA STANDARD
        writef(retvalue,"[PROMPT PAROLIERE]--> ");
    }
    
    return NULL;
}

void* trade(void* args){
    int comm_fd = *(int*) args;
    int retvalue;ssize_t n_read;
    char input_buffer[buff_size];
    while(1){
        pthread_mutex_lock(&user_mutex);
        tcflush(STDIN_FILENO, TCIFLUSH);
        SYSC(n_read,read(STDIN_FILENO,input_buffer,buff_size),"nella lettura da stdin");
        char* input = (char*)malloc(n_read+1);
        strncpy(input,input_buffer,n_read);
        input[n_read] = '\0';
        char* token = strtok(input," ");
        if (strcmp(token,"aiuto\n")==0){
            writef(retvalue,HELP_MESSAGE);
            //writef(retvalue,"ho unlockato\n");
            writef(retvalue,"[PROMPT PAROLIERE]--> ");
            pthread_mutex_unlock(&user_mutex);
            
            continue;
        }
        if (strcmp(token,"registra_utente")==0 || strcmp(token,"registra_utente\n")==0){
            token = strtok(NULL,"\n");
            if (token == NULL){
                writef(retvalue,"nome utente non valido\n");
                writef(retvalue,"[PROMPT PAROLIERE]--> ");
                continue;
            }
            
            if(only_space_string(token)==0){
                writef(retvalue,"nome utente vuoto\n");
                writef(retvalue,"[PROMPT PAROLIERE]--> ");
                continue;
            }
            if (strlen(token)>10){
                writef(retvalue,"nome utente troppo lungo\n");
                writef(retvalue,"[PROMPT PAROLIERE]--> ");
                continue;
            }
            if (token[0] == '\n'){
                writef(retvalue,"nome utente troppo corto\n");
                writef(retvalue,"[PROMPT PAROLIERE]--> ");
                continue;
            }
            //invio al server il messaggio con le credenziali per la registrazione
            type_sent = MSG_REGISTRA_UTENTE;
            Send_Message(comm_fd,token,MSG_REGISTRA_UTENTE);
            continue;
        }
        if (strcmp(token,"matrice\n")==0){
            //invio al server la richiesta della matrice
            type_sent = MSG_MATRICE;
             ;
            Send_Message(comm_fd,"matrice",MSG_MATRICE);
            continue;
        }
        if (strcmp(token,"p")==0){
            //tokenizzo la stringa per ottenere la parla inserita dall'utente
            token = strtok(NULL,"\n");
            //controllo che il token contenga qualcosa 
            
            if (token == NULL || token[0] == '\n' || strlen(token)<4){
                //se il token rientra in uno dei casi sopra sicuramente la parola non è valida, quindi non serve mandarla al server
                writef(retvalue,"inserisci una parola lunga almeno 4 parole\n");
                continue;
            }
                //rendo maiuscola la stringa
                Caps_Lock(token);
                //invio al server la parola
                type_sent = MSG_PAROLA;
                Send_Message(comm_fd,token,MSG_PAROLA);
            continue;
        }
        if (strcmp(token,"fine\n")==0){
            if (unwanted_termination == 1)break;
            //comunico al server che l'utente si sta disconnettendo
            Send_Message(comm_fd,"fine",MSG_CHIUSURA_CONNESSIONE);
            pthread_mutex_unlock(&user_mutex);
            SYST(retvalue,pthread_kill(bouncer,SIGUSR2),"nell'avviso verso il bouncer della chiusura");
            SYSC(retvalue,close(client_fd),"nella chiusura del client per scelta utente");
            return NULL;
        }
        if (strcmp(token,"score\n")==0){
            type_sent = MSG_PUNTEGGIO;
            Send_Message(comm_fd,"punti",MSG_PUNTEGGIO);
            continue;
        }
        if (strcmp(token,"classifica\n")==0){
            type_sent = MSG_PUNTI_FINALI;
            Send_Message(comm_fd,"classifica",MSG_PUNTI_FINALI);
            continue;
        }
        //stampa di default
        writef(retvalue,"comando non disponibile, digitare aiuto per una lista dettagliata\n");
        writef(retvalue,"[PROMPT PAROLIERE]--> ");
    }  
    return NULL;
}
//CONTROLLO SE LA STRINGA HA ALMENO UN CARATTERE
int only_space_string(char* string){
    int spaces = 0;
    for(int i =0;i<strlen(string);i++){
        if (string[i] == ' '){
            spaces++;
        }
    }
    if (spaces == strlen(string))return 0;
    else return -1;
}