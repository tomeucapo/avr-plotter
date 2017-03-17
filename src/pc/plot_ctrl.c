#include <fcntl.h>
#include <termios.h>
#include <stdio.h>
#include "plot_ctrl.h"

#define _POSIX_SOURCE 1


int open_dev_ser(char *nom_fitxer)
{
    int fd;
    fd = open(nom_fitxer,O_RDWR | O_NOCTTY);

    if (fd<0) {
        perror(nom_fitxer);
        return(-1);
    }

    tcgetattr(fd,&oldtio);
    bzero(&newtio,sizeof(newtio));

    newtio.c_cflag = B9600 | CS8 |CREAD | CLOCAL | HUPCL;
    cfsetospeed(&newtio, B9600);

    tcflush(fd,TCIFLUSH);
    tcsetattr(fd,TCSANOW,&newtio);
    return fd;
}

/*
int open_dev_ser(char *nom_dev)
{
    int fd;

    fd = open(nom_dev, O_RDWR | O_NOCTTY);

    if (fd<0) 
        perror(nom_dev);
    else {
        tcgetattr(fd,&oldtio);                     // Guarda els valors actuals del port
        bzero(&newtio,sizeof(newtio));             // Buida els valors per iniciar-los

	newtio.c_cflag |= CRTSCTS;
        newtio.c_cflag |= CS8;
        newtio.c_cflag |= CLOCAL;
	newtio.c_cflag |= CREAD;
       
        cfsetospeed(&newtio, B9600);

        newtio.c_iflag = ICRNL;           	// Converteix el caracter CR->NULL
        newtio.c_oflag = 0;               	// Sortida tipus RAW
        newtio.c_lflag = 0;
	
        tcflush(fd,TCIFLUSH);
        tcsetattr(fd,TCSANOW,&newtio);
    }
    return fd;
}
*/

void close_dev_ser(int fd_s)
{
     tcsetattr(fd_s,TCSANOW,&oldtio);
     close(fd_s);
}

int send_command(int fd_s, char *cmd)
{
    char buff[64];
    int i;
    
    sprintf(buff,"%s\n\n", cmd);
    i = write(fd_s, buff, strlen(buff));
   
    
    return i;	      
}

void usage()
{
     printf("Sintaxi:\n");
     printf("plot_ctrl -d [dev] [args]\n");
}


int main(int argc,char *argv[])
{
     int fd,i;
   
     if (argc == 1) {
	 usage();
	 exit(1);
     }
     
     if(strncmp(argv[1],"-d",2)==0) {
	if (argv[2]) {
            fd = open_dev_ser(argv[2]);
	    if (fd < 0)
		exit(1);
	    
	    send_command(fd,"H");
	    send_command(fd,"D");
            send_command(fd,"L 300 300");
	    send_command(fd,"U");
	    send_command(fd,"E");

	} else {
	    printf("ERROR 001: Falta especificar el dispositiu\n");
	    exit(1);
	}
     } else {
       printf("ERROR 001: Falta especificar el dispositiu\n");
       exit(1);
     }

     
     close_dev_ser(fd);
}
