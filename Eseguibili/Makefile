#Compilatore
CC = gcc

#Flag di compilazione 
CFLAGS = -Wall -pedantic -pthread -g

#Directories
HEADER = ../Header/
SORGENTE = ../Sorgente/

#Headers
HDRS1 = $(HEADER)macro.h
HDRS2 = $(HEADER)$(MATRICI).h
HDRS3 = $(HEADER)$(CODE).h
HDRS4 = $(HEADER)$(COMMS).h

#Eseguibili
SERVER = Server
CLIENT = Client
MATRICI = Matrix
CODE = Queue
COMMS = Communication

#File Oggetto
OBJS = $(SERVER).o
OBJC = $(CLIENT).o
OBJHM = $(MATRICI).o
OBJHQ = $(CODE).o
OBJHC = $(COMMS).o

#Sorgenti
SRCS = $(SORGENTE)$(SERVER).c
SRCC = $(SORGENTE)$(CLIENT).c
SRCM = $(SORGENTE)$(MATRICI).c
SRCQ = $(SORGENTE)$(CODE).c
SRCP = $(SORGENTE)$(COMMS).c

#Input
PORT = 20000
PORT2 = 30000
HOST = 127.0.0.1

#Test
SERVER_RUN = ./$(SERVER) $(HOST) $(PORT)
SERVER_RUN2 = ./$(SERVER) $(HOST) $(PORT2)
CLIENT_RUN = ./$(CLIENT) $(HOST) $(PORT)
CLIENT_RUN2 = ./$(CLIENT) $(HOST) $(PORT2)


#Target
all: $(SERVER) $(CLIENT) obj_clean

#Dipendenze file Eseguibili
$(SERVER): $(OBJS)
	@echo Server
	@#compilazione con Server.o e Matrix.o
	@$(CC) $(CFLAGS) $(OBJS) $(OBJHM) $(OBJHQ) $(OBJHC) -o $(SERVER)

$(CLIENT): $(OBJC)
	@echo Client
	@#compilazione con Client.o e Matrix.o
	@$(CC) $(CFLAGS) $(OBJC) $(OBJHM) $(OBJHQ) $(OBJHC) -o $(CLIENT)

#Dipendenze File Oggetto
#Server.o
$(OBJS): $(SRCS) $(HDRS1) $(HDRS2)
	@$(CC) $(CFLAGS) -c $(SRCS) $(SRCM) $(SRCQ) $(SRCP) $(HDRS1) $(HDRS2) $(HDRS3) $(HDRS4)
#Client.o
$(OBJC): $(SRCC) $(HDRS1) $(HDRS2) 
	@$(CC) $(CFLAGS) -c $(SRCC) $(SRCM) $(SRCQ) $(SRCP) $(HDRS1) $(HDRS2) $(HDRS3) $(HDRS4)
#Matrix.o
$(OBJHM): $(SRCM) $(HDRS2) $(HDRS3)
	$(CC) $(CFLAGS) -c $(SRCM) $(SRCQ) $(HDRS2) $(HDRS3)
#Queue.o
$(OBJHQ): $(SRCQ) $(HDRS3)
	$(CC) $(CFLAGS) -c $(SRCQ) $(HDRS3)
#Communication.o
$(OBJHC): $(SRCP) $(HDRS4)
	$(CC) $(CFLAGS) -c $(SRCP) $(HDRS4)

obj_clean:
	@rm -f $(OBJC) $(OBJS) $(OBJHM) $(OBJHQ) $(OBJHC)
	@echo file oggetto rimossi
clear: 
	@rm -f $(OBJC) $(OBJS) $(OBJHM) $(OBJHQ) $(OBJHC) $(SERVER) $(CLIENT)

run-server:
	@echo server port $(PORT)
	@$(SERVER_RUN)

run-server2:
	@echo server port $(PORT2)
	@$(SERVER_RUN2)
	
test-client:
	@echo client online
	@$(CLIENT_RUN)

test-client2:
	@echo client online
	@$(CLIENT_RUN2)

debug-client:
	@echo client online
	@gdb $(CLIENT_RUN)

debug-server:
	@echo server port $(PORT)
	@gdb $(SERVER_RUN)
