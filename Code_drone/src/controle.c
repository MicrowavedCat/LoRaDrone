#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include "../header/controle.h"
/* Nombre d'appareils déjà réservés */
#define RESERVED 256

/* Permet de vérifier la validation d'une transaction i2c d'adressage */
static void adressage(volatile int flux, unsigned char *configuration,
                      const unsigned short int taille){
  if((write(flux, configuration, taille) != taille)){
    printf("Erreur transaction i2c");
    exit(2);
  }
}

/* Permet de définir le fromat des valeurs des axes de l'accéléromètre ADXL345 */
volatile short int position(volatile short int axe,
                            unsigned char *data, const unsigned short int i){
  /*
  - On détermine le bit de poid faible de l'adresse données sur 32 bits,
  avec un ET bit à bit de 11, multiplié par le nombre d'appareils déjà réservés.
  - Puis on y ajoute le bit de poid fort en gardant l'identité.
  */
  axe = (data[i+1] & 0x03) * RESERVED + data[i];
  /* Si on dépasse, pour les données d'un axe, 2^9-1 = 511,
  on convertit les données sur 2^(9+1) = 2^10 = 1024. */
  if(axe > 511){ axe -= 1024; }
  return axe;
}

/* Fonction permettant de configurer, et de relever, 
les coordonnées de l'accéléromètre ADXL345. */
extern void i2c(void) {
  static volatile int fd;
  /* Ouverture du bus i2c en lecutre et écriture,
  permettant de connaitre les périphériques branchés. */
  if((fd = open(BUS, O_RDWR)) < 0){
    printf("Erreur communication\n");
    exit(1);
  }
  /*
  On effectue des opérations de communication d'entrée-sortie spécifiques à un périphérique,
  - Le flux de données du bus i2c permet d'assigner des adresses aux périphériques branchés.
  - On passe le périphérique en esclave, le système maitre-esclave, en i2c, 
  permet de déterminer un périphérique comme étant récépteur à son maitre
  (le récépteur étant en l'occurence l'ADXL345, et l'émetteur étant le Rapsberry Pi Zero).
  - L'adresse de l'accéléromètre est 53 (Cela se vérifie en faisant "sudo i2cdetect -y 1").
  */
  if(ioctl(fd, I2C_SLAVE, 0x53) < 0){
    printf("Erreur communication i2c\n");
    exit(2);
  }
  
  /* Registre initiale de configuration des adresses */
  static unsigned char config[2];
  
  config[0] = 0x2C; /* (0x2C = 44), determine le taux de bande passante */
  config[1] = 0x0A; /* (0x0A = 10), determine le débit de données de sortie à 100 Hz */
  /* Ecrirture de l'adresse 0x2C0A dans le flux de données. */
  adressage(fd, config, 2);

  config[0] = 0x2D; /* (0x2D = 45), determine le controle de puissance */
  config[1] = 0x08; /* (0x08 = 8), determine la fin d'adressage */
  /* Ecrirture de l'adresse 0x2D08 dans le flux de données. */
  adressage(fd, config, 2);

  config[0] = 0x31; /* 0x31 détermine le format de données */
  config[1] = 0x08; /* (0x08 = 8), determine la fin d'adressage */
  /* Ecrirture de l'adresse 0x3108 dans le flux de données. */
  adressage(fd, config, 2);
  
  while(1){
    /* Relever en permanance la stabilité du drone */
    sleep(1);
    /* Définition du registre de stockage d'adresse sur 32 bits */
    static unsigned char registre[1] = {0x32};
    /* Ecriture de l'adresse 0x32 */
    adressage(fd, registre, 1);
  
    static unsigned char data[6];
    /* Lecture de l'adresse 0x0A2C2D083108 écris dans le flux. */
    if(read(fd, data, 6) != 6){
      printf("Erreur lecture coordonnees\n");
      exit(3);
    }else{
      volatile short int x = position(x, data, 0);
      volatile short int y = position(y, data, 2);
      volatile short int z = position(z, data, 4);
      
      printf("X = %hd Y = %hd Z = %hd\n", x, y, z);
    }
  }
}
