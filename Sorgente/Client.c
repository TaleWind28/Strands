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
#include "../Header/Stack.h"
#include "../Header/Communication.h"

#define NUM_ROWS 4
#define NUM_COLUMNS 4
int client_fd;
void* bounce(void* args);
void* trade(void* args);
pthread_t bouncer,merchant,main_tid;
char* matrice;
int only_space_string(char* string);

void gestione_terminazione_errata(int signum) {
    int retvalue;
    switch (signum){
        case SIGINT:
            if(pthread_self()== main_tid){
                printf("sigint\n");
                pthread_kill(merchant,SIGUSR2);
                pthread_cancel(bouncer);
                pthread_join(merchant,NULL);
                SYSC(retvalue,close(client_fd),"chiusura del client");
                /*chiudo il socket*/
                exit(EXIT_SUCCESS);
                return;
            }
            else return;
        case SIGQUIT:
            if(pthread_self()== main_tid){
                printf("sigint\n");
                pthread_kill(merchant,SIGUSR2);
                pthread_cancel(bouncer);
                pthread_join(merchant,NULL);
                SYSC(retvalue,close(client_fd),"chiusura del client");
                /*chiudo il socket*/
                exit(EXIT_SUCCESS);
                return;
            }
            else return;

        case SIGUSR1:
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

    main_tid = pthread_self();

    SYST(retvalue,pthread_create(&bouncer,NULL,bounce,&client_fd),"nella creazione del bouncer");
    SYST(retvalue,pthread_create(&merchant,NULL,trade,&client_fd),"nella creazione del mercante");
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
                //stampo la matrice al client
                writef(retvalue,"Questa è la matrice su cui giocare\n");
                Print_Matrix(answer,4,4,'?');
                break;
            
            case MSG_TEMPO_ATTESA:
                //stampo al client la durata residua
                writef(retvalue,"Durata residua pausa ");
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
                
                if (strcmp("stringa vuota",answer)==0){
                    writef(retvalue,"Classifica non ancora disponibile");
                }else{
                    writef(retvalue,"classifica finale\n");
                    writef(retvalue,answer);
                }
                writef(retvalue,"\n");
                break;

            case MSG_ERR:
                writef(retvalue,answer);
                break;
            
            case MSG_OK:
                writef(retvalue,answer);
                break;

            case MSG_CHIUSURA_CONNESSIONE:
                writef(retvalue,answer);
                //mando un SIGUSR1 al thread principale
                SYST(retvalue,pthread_kill(merchant,SIGUSR1),"nell'avvisare il mercante della chiusura");
                return NULL;
        }
        //free(answer);
        writef(retvalue,"[PROMPT PAROLIERE]--> ");
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
        if (strcmp(token,"aiuto\n")==0){
            writef(retvalue,HELP_MESSAGE);
            writef(retvalue,"[PROMPT PAROLIERE]--> ");
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
            
            if (token[0] == '\n'){
                writef(retvalue,"nome utente troppo corto\n");
                continue;
            }
            //invio al server il messaggio con le credenziali per la registrazione
            Send_Message(comm_fd,token,MSG_REGISTRA_UTENTE);
            continue;
        }
        if (strcmp(token,"matrice\n")==0){
            //invio al server la richiesta della matrice
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
                Send_Message(comm_fd,token,MSG_PAROLA);
            continue;
        }
        if (strcmp(token,"fine\n")==0){
            //comunico al server che l'utente si sta disconnettendo
            Send_Message(comm_fd,"fine",MSG_CHIUSURA_CONNESSIONE);
            SYST(retvalue,pthread_kill(bouncer,SIGUSR2),"nell'avviso verso il bouncer della chiusura");
            SYSC(retvalue,close(client_fd),"nella chiusura del client per scelta utente");
            return NULL;
        }
        if (strcmp(token,"score\n")==0){
            Send_Message(comm_fd,"punti",MSG_PUNTEGGIO);
            continue;
        }
        if (strcmp(token,"classifica\n")==0){
            Send_Message(comm_fd,"classifica",MSG_PUNTI_FINALI);
            continue;
        }
        //stampa di default
        writef(retvalue,"comando non disponibile, digitare aiuto per una lista dettagliata\n");
        writef(retvalue,"[PROMPT PAROLIERE]-->");
    }  
    return NULL;
}

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