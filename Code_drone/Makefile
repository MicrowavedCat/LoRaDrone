### 1] But final du fichier Makefile ###
compilation : drone

### 2] Variables des fichiers d'extension ".o" à créer ###
OFILES = src/main.o \
         src/communication.o \
         src/controle.o \
	 src/moteur.o

# Compilation et arguments de compilation
CC = gcc -ansi -Ofast -flto

### 3] Dépedences à règles implicites des fichiers d'extension ".o" ###
communication.o : ../header/communication.h
controle.o : ../header/controle.h
moteur.o : ../header/moteur.h
main.o : communication.o controle.o

### 4] Dépedences précédentes appliquées par commandes ###
drone : $(OFILES)
	$(CC) -o drone $(OFILES) -lpthread -lwiringPi

### 5] Nettoyage des fichiers d'extension ".o" générés ###
clean :
	rm -f $(OFILES) drone
.PHONY : compilation clean

### 6] Exécution du programme par l'exécutable ###
run :
	./drone
.PHONY : run
