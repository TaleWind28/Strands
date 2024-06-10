#Compilatore
CC = gcc

#flag di compilazione
CFLAGS = -Wall -pedantic -pthread -g

#Directories
HEADER = ../Header/
SORGENTE = ../Sorgente/

#Header
HDRS = $(HEADER)$(MATRICI).h
HDRS2 = $(HEADER)$(COMMS).h
HDRS3 = $(HEADER)$(TRIE).h

#Eseguibili
MATRICI = Matrix
TEST = test_matrix
COMMS = Communication
TRIE = Trie
TEST3 = test_trie
TEST2 = test_comms

#File Sorgente
SRCM = $(SORGENTE)$(MATRICI).c
SRCT1 = $(TEST).c
SRCC = $(SORGENTE)$(COMMS).c
SRCT2 = $(TEST2).c
SRCT = $(SORGENTE)$(TRIE).c
SRCT3 = $(TEST3).c

#File Oggetto
OBJM = $(MATRICI).o
OBJT = $(TEST).o
OBJC = $(COMMS).o
OBJT2 = $(TEST2).o
OBJTT = $(TRIE).o
OBJT3 = $(TEST3).o

#Test
COMMON = ./$(TEST) 4 4 oveclihceolratou
UNCOMMON = ./$(TEST) 4 4 atlcioquadvesisbi

#Target
all: $(TEST) $(TEST2) $(TEST3) obj_clean
#Test_Matrix
$(TEST): $(OBJT)
	@echo matrici
	@$(CC) $(CFLAGS) $(OBJT) $(OBJM) $(OBJC) -o $(TEST)
#Test_Comms
$(TEST2): $(OBJT2)
	@echo Comms
	@$(CC) $(CFLAGS) $(OBJT2) $(OBJC) -o $(TEST2)

#Test_Trie
$(TEST3): $(OBJT3)
	@echo Trie
	@$(CC) $(CFLAGS) $(OBJT3) $(OBJTT) -o $(TEST3)

#Test_Trie.o
$(OBJT3): $(SRCT3) $(HDRS3)
	@echo creo test_comms.o
	$(CC) $(CFLAGS) -c $(SRCT3) $(SRCT) $(HDRS3)
	@echo creato

#Test_Comms.o
$(OBJT2): $(SRCT2) $(HDRS2)
	@echo creo test_comms.o
	$(CC) $(CFLAGS) -c $(SRCT2) $(SRCC) $(HDRS2)

#Test_Matrix.o
$(OBJT): $(SRCT1) $(HDRS) $(HDRS2)
	$(CC) $(CFLAGS) -c $(SRCT1) $(SRCM) $(SRCC) $(HDRS) $(HDRS2)

#Communication.o
$(OBJC): $(SRCC) $(HDRS2)
	$(CC) $(CFLAGS) -c $(SRCC) $(HDRS2)

#Trie.o
$(OBJTT): $(SRCM) $(HDRS3)
	$(CC) $(CFLAGS) -c $(SRCT) $(HDRS3)

obj_clean: 
	@rm -f $(OBJM) $(OBJT) $(OBJC) $(OBJT2) $(OBJT3) $(OBJTT)
	@echo file oggetto rimossi

clear:
	rm -f $(OBJM) $(OBJT) $(TEST) $(OBJC) $(OBJT2) $(TEST2)

test-w:
	# $(COMMON) chilo
	# $(COMMON) loto
	# $(COMMON) oleato
	# $(COMMON) olive
	# $(COMMON) ruolo
	# $(COMMON) urlo
	# $(COMMON) vecchio
	# $(COMMON) vile
	# $(COMMON) voli
	# $(COMMON) ruota
	$(UNCOMMON) CASE ../'File Testuali'/Matrici.txt
	$(UNCOMMON) CASI ../'File Testuali'/Matrici.txt
	$(UNCOMMON) BEVO ../'File Testuali'/Matrici.txt
	$(UNCOMMON) VOTA ../'File Testuali'/Matrici.txt
	$(UNCOMMON) QuASI ../'File Testuali'/Matrici.txt

	
