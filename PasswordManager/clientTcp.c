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
  char exit_msg[BUFFSIZE] = "____Password Manager____";

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

  if (read(sd, recv_msg, 200) < 0)
  {
    perror("Eroare la citire de la server.\n");
    return errno;
  }
  printf("%s\n", recv_msg);

  while (1)
  {
    bzero(sent_msg, 1000);
    read(0, sent_msg, 1000);
    int n = strlen(sent_msg);
    if (n != 1)
      n--;
    if (write(sd, sent_msg, n) <= 0)
    {
      perror("Eroare la scriere spre server.\n");
      return errno;
    }
    bzero(recv_msg, BUFFSIZE);

    if (read(sd, recv_msg, BUFFSIZE) < 0)
    {
      perror("Eroare la citire de la server.\n");
      return errno;
    }
    printf("%s\n", recv_msg);
    if (!strcmp(recv_msg, "Exiting"))
    {
      close(sd);
      printf("%s\n", exit_msg);
      exit(0);
    }
  }
  return 0;

}
