#include "../header/controle.h"

void i2c(void) {
  int fd;
  if ((fd = open(BUS, O_RDWR)) < 0){
    perror("Erreur communication ");
    exit(1);
  }
  /* Contrôle d'entrée-sortie vers le système esclave,
  7 bits que l’on complète par un 1 ou 0 final selon que le maître lit ou écrit sur cet esclave.
  L'adresse est donc 0x53 soit 83 ou 1010011.
  L'utilisation d'une adresse qui n’est pas déjà utilisée sur les devices est nécessaires. */
  ioctl(fd, I2C_SLAVE, 0x53);
  char config[2];
  /* (0x2C = 44) sélectionne du registre de taux de bande passante */
  config[0] = 0x2C;
  /* (0x0A = 10) correpond au débit de données de sortie (100 Hz) */
  config[1] = 0x0A;
  /* Ecrire la configuration dans le flux de taille 2 */
  write(fd, config, 2); /* Et 44 + 10 < 83 */

  /* (0x2D = 45), configure le registre de controle de puissance */
  config[0] = 0x2D;
  /* (0x08 = 8), mise en veille automatique */
  config[1] = 0x08;
  /* Ecrirture de la configuration dans le flux de taille 2 */
  write(fd, config, 2); /* Et 45 + 8 < 83 */

  /* (0x31 = 49), sélectionne le format de données */
  config[0] = 0x31;
  /* (0x08 = 8), autotest désactivé, plage + ou - équivalente à 2g */
  config[1] = 0x08;
  write(fd, config, 2); /* Et 49 + 8 < 83 */
  usleep(1000000);

  /* Reservation d'une adresse de 32 bits */
  char registre[1] = {0x32};
  write(fd, registre, 1);
  /* Allocation des 6 octets de données du registre à lire */
  char data[6];
  /* Problème de lecture du flux */
  if(read(fd, data, 6) != 6){
    perror("Erreur lecture : ");
    exit(2);
  }else{
    /* --> lsb x, msb x
    Prend le bit de poid faible, effectue un ET bit à bit,
    puis prend le bit de poid fort stocké dans x. */
    short int x = ((data[1] & 0x03) * 256 + data[0]);
    /* On parcours la valeur des bits de 0 à 9, on a 2^9-1 = 511 */
    if(x > 511){ x -= 1024; }
	  
    short int y = ((data[3] & 0x03) * 256 + data[2]);
    if(y > 511){ y -= 1024; }
	  
    short int z = ((data[5] & 0x03) * 256 + data[4]);
    if(z > 511){ z -= 1024; }
	  
    printf("Controle X : %hd\nControle Y : %hd\nControle Z : %hd\n", x, y, z);
  }
}
