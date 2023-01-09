#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

#define BUFFSIZE 1024

/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main (int argc, char *argv[])
{
  int sd;			// descriptorul de socket
  struct sockaddr_in server;	// structura folosita pentru conectare 
  char sent_msg[BUFFSIZE], recv_msg[BUFFSIZE];
  char exit_msg[BUFFSIZE] = "____Exiting Password Manager____";

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
    {
      printf ("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
      return -1;
    }

  /* stabilim portul */
  port = atoi (argv[2]);

  /* cream socketul */
  if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
      perror ("Eroare la socket().\n");
      return errno;
    }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons (port);
  
  /* ne conectam la server */
  if (connect (sd, (struct sockaddr *) &server,sizeof (struct sockaddr)) == -1)
    {
      perror ("[client]Eroare la connect().\n");
      return errno;
    }

    while (1) {
        /* citirea mesajului */
        bzero (sent_msg, BUFFSIZE);
        printf ("[client]Introduceti o comanda: ");
        fflush (stdout);
        read (0, sent_msg, BUFFSIZE);

        int n = strlen(sent_msg);
        if ( n != 1)
            n--;
        
        /* trimiterea mesajului la server */
        if (write (sd, sent_msg, n) <= 0)
            {
            perror ("[client]Eroare la write() spre server.\n");
            return errno;
            }

        bzero(recv_msg, BUFFSIZE);
        /* citirea raspunsului dat de server 
            (apel blocant pina cind serverul raspunde) */
        if (read (sd, recv_msg, BUFFSIZE) < 0)
            {
            perror ("[client]Eroare la read() de la server.\n");
            return errno;
            }
        /* afisam mesajul primit */
        printf ("[client]Mesajul de la server: %s\n", recv_msg);

        //strip
        recv_msg[strcspn(recv_msg, "\n")] = 0;
        if (!strcmp(recv_msg, exit_msg))
        {
            close(sd);
            exit(0);
        }
    }
  return 0;
}
