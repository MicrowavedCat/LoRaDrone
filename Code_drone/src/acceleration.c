#include "../header/acceleration.h"

/* Tableau de valeurs d'acceleration lineaire */
extern volatile short int acceleration[3];

/**** 
* Permet de verifier la validation d'une transaction i2c d'adressage *
****/
static void adressage(volatile int flux, unsigned char *config,
                      const unsigned short int espace){
  if(write(flux, config, espace) != espace){
    puts("Erreur transaction i2c");
    exit(2);
  }
}

/****
* Permet de definir le format des valeurs des axes de l'accelerometre ADXL345 *
****/
static volatile short int position(volatile short int axe,
                            unsigned char *data, const unsigned short int i){
  /*
  - On determine le bit de poid faible de l'adresse donnees sur 32 bits,
  avec un ET bit a bit de 11, multiplie par l'espace d'appareils reserves de 256.
  - Puis on y ajoute le bit de poid fort en gardant l'identite.
  */
  axe = (data[i+1] & 0x03) * 256 + data[i];
  /* Si on depasse, pour les donnees d'un axe, 2^9 = 512 valeurs soit de 0 à 511,
  on convertit les donnees sur 2^(9+1) = 2^10 = 1024. */
  if(axe > 511){ axe -= 1024; }
  return axe;
}

/**** 
* Fonction permettant de configurer, et de relever, *
* les coordonnees de l'accelerometre ADXL345. * 
****/
extern void celerite(void){
  static volatile int fd;
  /* Ouverture du bus i2c en lecutre et ecriture,
  permettant de connaitre les peripheriques branches. */
  if((fd = open(BUS, O_RDWR)) < 0){
    puts("Erreur communication");
    exit(1);
  }
  /*
  On effectue des operations de communication d'entree-sortie specifiques à un peripherique,
  - Le flux de donnees du bus i2c permet d'assigner une adresse aux peripheriques branches.
  - On passe le peripherique en esclave : le systeme maitre-esclave permet, en i2c, 
  de determiner un peripherique comme etant recepteur a son maitre, qui est emetteur
  (le recepteur etant en l'occurence l'ADXL345, et l'emetteur etant le Rapsberry Pi Zero).
  - L'adresse de l'accelerometre est 53 (Cela se verifie en faisant "sudo i2cdetect -y 1").
  */
  if(ioctl(fd, I2C_SLAVE, 0x53) < 0){
    puts("Erreur communication i2c");
    exit(2);
  }
  /* Registre initiale de configuration des adresses */
  static unsigned char config[2];
  
  config[0] = 0x2C; /* (0x2C = 44), determine le taux de bande passante */
  config[1] = 0x0A; /* (0x0A = 10), determine le debit de donnees de sortie à 100 Hz */
  /* Ecrirture de l'adresse 0x2C0A dans le flux de donnees. */
  adressage(fd, config, 2);

  config[0] = 0x2D; /* (0x2D = 45), determine le controle de puissance */
  config[1] = 0x08; /* (0x08 = 8), determine la fin d'adressage */
  /* Ecrirture de l'adresse 0x2D08 dans le flux de donnees. */
  adressage(fd, config, 2);

  config[0] = 0x31; /* 0x31 determine le format de donnees */
  config[1] = 0x08; /* (0x08 = 8), determine la fin d'adressage */
  /* Ecrirture de l'adresse 0x3108 dans le flux de donnees. */
  adressage(fd, config, 2);
  
  while(1){
    /* Relever en permanance l'acceleration lineaire du drone */
    sleep(1);
    /* Definition du registre de stockage d'adresse sur 32 bits */
    static unsigned char registre[1] = {0x32};
    /* Ecriture de l'adresse 0x32 */
    adressage(fd, registre, 1);
    static unsigned char data[6];
    /* Lecture de l'adresse 0x0A2C2D083108 ecris dans le flux. */
    if(read(fd, data, 6) != 6){
      puts("Erreur lecture coordonnees");
      exit(3);
    }else{
      /* Stockage des valeurs de chaque axe x, y et z */
      volatile unsigned short int offset = 0;
      for(volatile unsigned short int i=0; i<3; i++){
        acceleration[i] = position(acceleration[i], data, offset);
        offset += 2;
      }
    }
  }
}
