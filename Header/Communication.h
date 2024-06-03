/*STANDARDIZZAZIONE DEI MESSAGGI PER COMUNCAZIONE TRA CLIENT E SERVER*/
#define MSG_OK 'K'
#define MSG_ERR 'E'
#define MSG_REGISTRA_UTENTE 'R'
#define MSG_MATRICE 'M'
#define MSG_TEMPO_PARTITA 'T'
#define MSG_TEMPO_ATTESA 'A'
#define MSG_PAROLA 'W'
#define MSG_PUNTI_FINALI 'F'
#define MSG_PUNTI_PAROLA 'P'
#define MSG_PUNTEGGIO 'S'
#define MSG_CHIUSURA_CONNESSIONE 'C'
#define RULES "Nel gioco del paroliere si indovinano parole dalla matrice e vengono assegnati dei punti in base alla lunghezza della parola.\nNella nostra versione si accettano solo parole di lunghezza superiore a 3 e vista la scarsità di parole con la Q nella matrice quest'ultima sarà sempre accompagnata da una U che conterà come lettera successiva, ad esempio la parola QUASI può essere composta in questo modo e vale 5 punti.\n"
#define WELCOME_MESSAGE "Ciao, benvenuto nel gioco del paroliere ,per prima cosa se non lo hai già fatto resgistrati mediante il comando registra_utente seguito dal tuo nome, per unirti ad una partita in corso o aspettare l'inizio di una nuova.\nDurante la partita potrai usare i seguenti comandi:\np <parola> \tper indovinare una parola presente nella matrice che vedi a schermo\n matrice\tper visualizzare a schermo la matrice ed il tempo residuo di gioco o di attesa\naiuto\tche ti mostra i comandi a te disponibili\nscore\tche ti mostra il tuo punteggio attuale\nfine\tche ti fa uscire dalla partita in corso\n"
#define HELP_MESSAGE "Puoi utilizzare i seguenti comandi:p <parola> \tper indovinare una parola presente nella matrice che vedi a schermo\nmatrice\tper visualizzare a schermo la matrice ed il tempo residuo di gioco o di attesa\naiuto\tche ti mostra i comandi a te disponibili\nscore\tche ti mostra il tuo punteggio attuale\nfine\tche ti fa uscire dalla partita in corso\n"


/*RENDO MAIUSCOLA UNA STRINGA*/
void Caps_Lock(char* string);
//invio un messaggio
void Send_Message(int comm_fd,char* payload,char type);
//ricevo un messaggio
char* Receive_Message(int comm_fd,char* type);