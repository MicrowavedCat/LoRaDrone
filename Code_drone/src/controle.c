#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include "../header/controle.h"
/* Les 256 numéros d'appareil mineurs sont réservés à i2c. */
#define RESERVED 256

/* Fonction permettant de configurer, et de relever, 
les coordonnées de l'accéléromètre ADXL345. */
extern const void i2c(void) {
  static volatile int fd;
  /* Ouverture du bus i2c en lecutre et écriture */
  if ((fd = open(BUS, O_RDWR)) < 0){
    printf("Erreur communication\n");
    exit(1);
  }
  /* ioctl([1],[2],[3]) est un appel système particulier, 
  permettant d'effectuer des opérations de communication d'entrée-sortie spécifiques à un périphérique,
  présentement ici un accéléromère branché sur des GPIOs d'un Rapsberry Pi0 :
  -> L'argument [1] est un descripteur de flux de données, en l'occurence pour le fichier "/dev/i2c-1",
  permettant d'accéder à tous les appareils d'un adaptateur depuis l'espace utilisateur,
  car habituellement les périphériques i2c sont contrôlés par un driver du noyau (Kernel).
  -> L'argument [2] est une requête relative à un périphérique.   
  -> L'argument [3] est est l'adresse par défault de connection de l'ADXL345,
  que l'on peut touver dans le chemin d'accés "/sys/class/i2c-adapter/", on obtient 53. */
  if(ioctl(fd, I2C_SLAVE, 0x53) < 0){
    printf("Erreur communication i2c\n");
    exit(2);
  }
  static unsigned char config[2];
  /* (0x2C = 44) sélectionne du registre de taux de bande passante */
  config[0] = 0x2C;
  /* (0x0A = 10) correpond au débit de données de sortie (100 Hz) */
  config[1] = 0x0A;
  /* Ecrirture de l'adresse 0x0A2C dans le flux de données. */
  if(write(fd, config, 2) != 2){
    printf("Erreur de transaction i2c\n");
    exit(2);
  }

  /* (0x2D = 45), configure le registre de controle de puissance */
  config[0] = 0x2D;
  /* (0x08 = 8), mise en veille automatique */
  config[1] = 0x08;
  /* Ecrirture de l'adresse 0x2D08 dans le flux de données. */
  if(write(fd, config, 2) != 2){
    printf("Erreur de transaction i2c\n");
    exit(2);
  }

  /* (0x31 = 49), sélectionne le format de données */
  config[0] = 0x31;
  /* (0x08 = 8), autotest désactivé, plage + ou - équivalente à 2g */
  config[1] = 0x08;
  /* Ecrirture de l'adresse 0x3108 dans le flux de données. */
  if(write(fd, config, 2) != 2){
    printf("Erreur de transaction i2c\n");
    exit(2);
  }
  usleep(1000000);
  
  static const unsigned char registre[1] = {0x32};
  write(fd, registre, 1);
  /* Lecture de l'adresse 0x0A2C2D083108 écris dans le flux. */
  unsigned char data[6];
  /* Problème de transaction i2c. */
  if(read(fd, data, 6) != 6){
    printf("Erreur de transaction i2c\n");
    exit(3);
  /* L'accéléromètre utilisé ici est sur 3 axes, 
  on effectue alors des opération pour les coordonnées x, y et z 
  --> lsb axe = Par du bit le moins significatif, effectue un ET bit à bit avec 11,
  --> msb axe = puis on y ajoute le bit le plus significatif. */
  }else{
    volatile short int x = ((data[1] & 0x03) * RESERVED + data[0]);
    /* Si l'on dépasse, pour les données d'un axe, 2^9-1 = 511,
    on convertit les données sur 10 bits [2^(9+1) = 2^10 = 1024 bits] */
    if(x > 511){ x -= 1024; }
    
    volatile short int y = ((data[3] & 0x03) * RESERVED + data[2]);
    if(y > 511){ y -= 1024; }
    
    volatile short int z = ((data[5] & 0x03) * RESERVED + data[4]);
    if(z > 511){ z -= 1024; }
    
    printf("-> Axe X : %hd\n-> Axe Y : %hd\n-> Axe Z : %hd\n", x, y, z);
  }
}
