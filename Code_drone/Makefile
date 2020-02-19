### 1] But final du fichier Makefile ###
compilation : drone

### 2] Variables des fichiers d'extension ".o" a creer ###
OFILES = src/main.o \
         src/communication.o \
	 src/moteur.o \
	 src/acceleration.o \
	 src/distance.o

# Compilation et arguments de compilation
CC = gcc -Ofast -flto

### 3] Depedences a regles implicites des fichiers d'extension ".o" ###
communication.o : ../header/communication.h
moteur.o : ../header/moteur.h ../header/communication.h ../header/acceleration.h
acceleration.o : ../header/acceleration.h
distance.o : ../header/distance.h
main.o : communication.o

### 4] Depedences precedentes appliquees par commandes ###
drone : $(OFILES)
	$(CC) -o drone $(OFILES) -lpthread -lwiringPi

### 5] Nettoyage des fichiers d'extension ".o" generes ###
clean :
	rm -f $(OFILES) drone
.PHONY : compilation clean

### 6] Execution du programme par l'executable ###
run :
	sudo ./drone
.PHONY : run
