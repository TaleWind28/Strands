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
#define MSG_CHIUSURA_CONNESSIONE 'C'

/*RENDO MAIUSCOLA UNA STRINGA*/
void Caps_Lock(char* string);
//invio un messaggio
void Send_Message(int comm_fd,char* payload,char type);
//ricevo un messaggio
char* Receive_Message(int comm_fd,char* type);