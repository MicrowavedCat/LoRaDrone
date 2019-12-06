### 1] But final du fichier Makefile ###
compilation : com

### 2] Variables des fichiers d'extension ".o" à créer ###
OFILES = src/main.o \
         src/communication.o

# Argument de compilation
CFLAGS = -Ofast -flto

### 3] Dépedences à règles implicites des fichiers d'extension ".o" ###
communication.o : ../header/communication.h
main.o : communication.o

### 4] Dépedences précédentes appliquées par commandes ###
com : $(OFILES)
	gcc $(CFLAGS) -o com $(OFILES) -lpthread -lwiringPi

### 5] Nettoyage des fichiers d'extension ".o" générés ###
clean :
	-rm -f $(OFILES) com
.PHONY : compilation clean

### 6] Exécution du programme par l'exécutable ###
run :
	./com
.PHONY : run
