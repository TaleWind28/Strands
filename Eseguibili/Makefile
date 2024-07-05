#Compilatore
CC = gcc

#Flag di compilazione 
CFLAGS = -Wall -pedantic -pthread -g -O0

#Directories
HEADER = ../Header/
SORGENTE = ../Sorgente/

#Headers
HDRS1 = $(HEADER)macro.h
HDRS2 = $(HEADER)$(MATRICI).h
HDRS3 = $(HEADER)$(PILE).h
HDRS4 = $(HEADER)$(COMMS).h
HDRS5 = $(HEADER)$(TRIE).h
HEADER-FILES = $(HDRS1).gch $(HDRS2).gch $(HDRS3).gch $(HDRS4).gch $(HDRS5).gch

#Eseguibili
SERVER = Server
CLIENT = Client
MATRICI = Matrix
PILE = Stack
COMMS = Communication
TRIE = Trie

#File Oggetto
OBJS = $(SERVER).o
OBJC = $(CLIENT).o
OBJHM = $(MATRICI).o
OBJHQ = $(PILE).o
OBJHC = $(COMMS).o
OBJHT = $(TRIE).o
OBJECT-FILES = $(OBJC) $(OBJS) $(OBJHM) $(OBJHQ) $(OBJHC) $(OBJHT)
#Sorgenti
SRCS = $(SORGENTE)$(SERVER).c
SRCC = $(SORGENTE)$(CLIENT).c
SRCM = $(SORGENTE)$(MATRICI).c
SRCQ = $(SORGENTE)$(PILE).c
SRCP = $(SORGENTE)$(COMMS).c
SRCT = $(SORGENTE)$(TRIE).c

#Input
PORT = 20000
PORT2 = 30000
PORT3 = 40000
HOST = 127.0.0.1

#Test
SERVER_RUN = ./$(SERVER) $(HOST) $(PORT) --matrici ../Text/Matrici.txt --diz ../Text/Dizionario.txt 
SERVER_RUN2 = ./$(SERVER) $(HOST) $(PORT2)  --durata 2 --seed 500
CLIENT_RUN = ./$(CLIENT) $(HOST) $(PORT)
CLIENT_RUN2 = ./$(CLIENT) $(HOST) $(PORT2)
SCORER_RUN = ./$(SERVER) $(HOST) $(PORT3) --durata 0.003 --matrici ../Text/Matrici.txt
SCORER_CLIENT = ./$(CLIENT) $(HOST) $(PORT3)

#Target
all: $(SERVER) $(CLIENT) obj_clean other_clean

#Dipendenze file Eseguibili
$(SERVER): $(OBJS)
	@echo Server
	@#compilazione con Server.o e Matrix.o
	@$(CC) $(CFLAGS) $(OBJS) $(OBJHM) $(OBJHQ) $(OBJHC) $(OBJHT) -o $(SERVER)

$(CLIENT): $(OBJC)
	@echo Client
	@#compilazione con Client.o e Matrix.o
	@$(CC) $(CFLAGS) $(OBJC) $(OBJHM) $(OBJHQ) $(OBJHC) $(OBJHT) -o $(CLIENT)

#Dipendenze File Oggetto
#Server.o
$(OBJS): $(SRCS) $(HDRS1) $(HDRS2)
	@$(CC) $(CFLAGS) -c $(SRCS) $(SRCM) $(SRCQ) $(SRCP) $(SRCT) $(HDRS1) $(HDRS2) $(HDRS3) $(HDRS4) $(HDRS5) 
#Client.o
$(OBJC): $(SRCC) $(HDRS1) $(HDRS2) 
	@$(CC) $(CFLAGS) -c $(SRCC) $(SRCM) $(SRCQ) $(SRCP) $(SRCT) $(HDRS1) $(HDRS2) $(HDRS3) $(HDRS4) $(HDRS5) 
#Matrix.o
$(OBJHM): $(SRCM) $(HDRS2) $(HDRS3)
	$(CC) $(CFLAGS) -c $(SRCM) $(SRCQ) $(HDRS2) $(HDRS3)
#Queue.o
$(OBJHQ): $(SRCQ) $(HDRS3)
	$(CC) $(CFLAGS) -c $(SRCQ) $(HDRS3)
#Communication.o
$(OBJHC): $(SRCP) $(HDRS4)
	$(CC) $(CFLAGS) -c $(SRCP) $(HDRS4)
#Trie.o
$(OBJHT): $(SRCT) $(HDRS5)
	$(CC) $(CFLAGS) -c $(SRCT) $(HDRS5)

obj_clean:
	@rm -f $(OBJECT-FILES)
	@echo file oggetto rimossi
other_clean:
	@rm -f $(HEADER-FILES)

clear: 
	@rm -f $(OBJECT-FILES) $(SERVER) $(CLIENT)

run-server:
	@echo server port $(PORT)
	@$(SERVER_RUN)

run-server2:
	@echo server port $(PORT2)
	@$(SERVER_RUN2)

run-scorer:
	@echo test-scorer on port 40000
	@$(SCORER_RUN)
test-scorer:
	@echo client-scorer
	@$(SCORER_CLIENT)
test-client:
	@echo client online
	@$(CLIENT_RUN)

test-client2:
	@echo client online
	@$(CLIENT_RUN2)