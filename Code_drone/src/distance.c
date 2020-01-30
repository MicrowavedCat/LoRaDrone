#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <time.h>
/* Librairie d'accès aux appel systèmes sur la mémoire */
#include <sys/mman.h>

#define ENTREE(pin) *(gpio+((pin)/10)) &= ~(7<<(((pin)%10)*3))
#define SORTIE(pin) *(gpio+((pin)/10)) |= (1<<(((pin)%10)*3))
#define CONFIG(pin) *(gpio+7) = 1<<(pin)
#define REMOVE(pin) *(gpio+10) = 1<<(pin)
#define GET *(gpio+13)

#define OFFSET 0x3F200000L
#define PAGE 4096
#define BLOCK 4096

#define GPIO2 3 /* Emetteur */
#define GPIO3 5 /* Récépteur */

static volatile int fd = 0;
/* Mapping des GPIOs sur la mémoire */
static unsigned char *projection = NULL;
/* Entier signé de 32 bits */
static volatile uint32_t *gpio = NULL;
static short int pin = 0;

/* Echo de l'onde sonore, émission + récéption */
static void ping(long temps){
  static struct timespec delai;
  /* temps en secondes */
  delai.tv_sec = temps / (1000 * 1000);
  /* temps en nano-secondes */
  delai.tv_nsec = (temps % (1000 * 1000)) * 1000;
  nanosleep(&delai, NULL);
}

static void config_memoire(const char *argv[]){
  static volatile char *addr  = NULL;
  if((fd = open("/dev/mem", O_RDWR|O_SYNC)) < 0) {
    printf("Conseil : \"sudo %s\"\n", argv[0]);
    exit(1);
  }
  if((addr = malloc(BLOCK + (PAGE-1))) == NULL) {
    puts("Allocation inacessible");
    exit(1);
  }
  if((unsigned long)addr % PAGE)
    addr += PAGE - ((unsigned long)addr % PAGE);
  /* Nouvelle projection dans l'espace d'adressage virtuel du processus appelant */
  projection = (unsigned char *)mmap(
    /* Adresse de départ pour placer projection */
    (caddr_t)addr, BLOCK, /* Longueur de la projection */
    /* Lire le contenu de la zone mémoire, et y écrire */
    PROT_READ | PROT_WRITE,
    /* Modifications de la projection visibles par les autres processus,
    qui projettent le fichier dev/mem, en plaçant la projection exactement à l'adresse donnée */
    MAP_SHARED | MAP_FIXED,
    fd, OFFSET
  );
  if((long)projection < 0) {
    puts("Memoire inacessible");
    exit(1);
  }
  gpio = (volatile uint32_t *)projection;
}

/* Sortie de la mémoire et fermeture des mappings */
static void sortie_memoire(){
  static volatile int mapping;
  mapping = munmap(projection, BLOCK);
  if(mapping == -1){
    puts("Erreur");
    exit(2);
  }
  mapping = close(fd);
  exit(0);
}

extern const unsigned short int main(unsigned short int argc, 
				     char const *argv[]){
  config_memoire(argv);

  static struct timespec impulsion, reception, portee;
  static volatile double distance, echo;

  ENTREE(GPIO2);
  SORTIE(GPIO2);
    
  ENTREE(GPIO3);
  ping(500000);
    
  /* Envoyer toutes les 10 nano-secondes à l'émetteur */
  CONFIG(GPIO2);
  ping(10);
  REMOVE(GPIO2);
    
  clock_gettime(CLOCK_REALTIME, &impulsion);
  while((GET >> GPIO3) & 0);
  /* Boucler tant que le bit 23 est à 0, puis enregistrer le temps. */
  clock_gettime(CLOCK_REALTIME, &impulsion);    

  clock_gettime(CLOCK_REALTIME, &reception);
  while((GET >> GPIO3) & 1);
  /* Boucler tant que le bit 23 est à 1, puis enregistrer le temps. */
  clock_gettime(CLOCK_REALTIME, &reception);
    
  if((reception.tv_nsec - impulsion.tv_nsec) < 0) {
    portee.tv_sec = reception.tv_sec - impulsion.tv_sec - 1;
    portee.tv_nsec = 1000000000 + reception.tv_nsec - impulsion.tv_nsec;
  }else{
     portee.tv_sec = reception.tv_sec - impulsion.tv_sec;
     portee.tv_nsec = reception.tv_nsec - impulsion.tv_nsec;
  }
  echo = ((portee.tv_sec*1000000000) + (portee.tv_nsec));
  echo /= 1000000000;
  /* calcul de la distance en cm */
  distance = (echo*17150);
  printf("Distance: %.2f cm\n", distance);
   
  sortie_memoire();
}
