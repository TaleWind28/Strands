#Compilatore
CC = gcc

#flag di compilazione
CFLAGS = -Wall -pedantic -pthread -g

#Directories
HEADER = ../Header/
SORGENTE = ../Sorgente/

#Header
HDRS = $(HEADER)$(MATRICI).h

#Eseguibili
MATRICI = Matrix
TEST = test_matrix

#File Sorgente
SRCM = $(SORGENTE)$(MATRICI).c
SRCT = $(TEST).c

#File Oggetto
OBJM = $(MATRICI).o
OBJT = $(TEST).o

#Test
COMMON = ./$(TEST) 4 4 oveclihceolratou
#Target
all: $(TEST) obj_clean

$(TEST): $(OBJT)
	@echo Client
	@#compilazione con Client.o e Matrix.o
	$(CC) $(CFLAGS) $(OBJT) $(OBJM) -o $(TEST)

$(OBJT): $(SRCT) $(HDRS)
	$(CC) $(CFLAGS) -c $(SRCT) $(SRCM) $(HDRS)

$(OBJM): $(SRCM) $(HDRS)
	$(CC) $(CFLAGS) -c $(SRCM) $(HDRS)

obj_clean: 
	@rm -f $(OBJM) $(OBJT)
	@echo file oggetto rimossi

clear:
	rm -f $(OBJM) $(OBJT) $(TEST)

test-w:
	$(COMMON) chilo
	$(COMMON) loto
	$(COMMON) oleato
	$(COMMON) olive
	$(COMMON) ruolo
	$(COMMON) urlo
	$(COMMON) vecchio
	$(COMMON) vile
	$(COMMON) voli
	$(COMMON) ruota


	
