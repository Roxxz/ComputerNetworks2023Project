#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <arpa/inet.h>

/* portul folosit */
#define PORT 2024
#define BUFFSIZE 1024

/* codul de eroare returnat de anumite apeluri */
extern int errno;

int main ()
{
    struct sockaddr_in server;	// structura folosita de server
    struct sockaddr_in from;
    char sent_msg[BUFFSIZE], recv_msg[BUFFSIZE];
    char welcome_msg[BUFFSIZE] = "____Password Manager____";
    int sd;			//descriptorul de socket

    /* crearea unui socket */
    if ((sd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
    {
    	perror ("[server]Eroare la socket().\n");
    	return errno;
    }

    /* pregatirea structurilor de date */
    bzero (&server, sizeof (server));
    bzero (&from, sizeof (from));

    /* umplem structura folosita de server */
    /* stabilirea familiei de socket-uri */
    server.sin_family = AF_INET;
    /* acceptam orice adresa */
    server.sin_addr.s_addr = htonl (INADDR_ANY);
    /* utilizam un port utilizator */
    server.sin_port = htons (PORT);

    /* atasam socketul */
    if (bind (sd, (struct sockaddr *) &server, sizeof (struct sockaddr)) == -1)
    {
    	perror ("[server]Eroare la bind().\n");
    	return errno;
    }

    /* punem serverul sa asculte daca vin clienti sa se conecteze */
    if (listen (sd, 1) == -1)
    {
    	perror ("[server]Eroare la listen().\n");
    	return errno;
    }

    /* servim in mod concurent clientii... */
    while (1)
    {
        int client;
    	int length = sizeof (from);

    	printf ("[server]Asteptam la portul %d...\n",PORT);
    	fflush (stdout);

    	/* acceptam un client (stare blocanta pina la realizarea conexiunii) */
    	client = accept (sd, (struct sockaddr *) &from, &length);

    	/* eroare la acceptarea conexiunii de la un client */
    	if (client < 0)
    	{
    		perror ("[server]Eroare la accept().\n");
    		continue;
    	}

    	int pid;
    	if ((pid = fork()) == -1) {
    		close(client);
    		continue;
    	} else if (pid > 0) {
    		// parinte
    		close(client);
    		while(waitpid(-1,NULL,WNOHANG));
    		continue;
    	} else if (pid == 0) {
    		// copil
            int is_logged = 0, admin = 0, perm = 0;
    		close(sd);

            //file handling
            // FILE *fp;
            // char filename[] = "master_users.txt";
            // char line[50];

            // fp = fopen(filename, "r");
            // if (!fp)
            // {
            //         perror("File does not exist!");
            //         exit(0);
            // }

            while (1)
            {
                /* s-a realizat conexiunea, se astepta mesajul */
                bzero (recv_msg, BUFFSIZE);
                printf ("[server]Asteptam mesajul...\n");
                fflush (stdout);
                
                /* citirea mesajului */
                if (read (client, recv_msg, BUFFSIZE) <= 0)
                {
                    perror ("[server]Eroare la read() de la client.\n");
                    close (client);	/* inchidem conexiunea cu clientul */
                    continue;		/* continuam sa ascultam */
                }

                printf ("[server]Mesajul a fost receptionat...%s\n", recv_msg);
                //strip
                recv_msg[strcspn(recv_msg, "\n")] = 0;

                if (!strcmp(recv_msg, "quit"))
                    {
                        bzero(sent_msg, BUFFSIZE);
                        printf(sent_msg, "Exiting...");
                        break;
                    }
                else if (strstr(recv_msg, "login") == recv_msg)
                    {
    		            bzero(sent_msg, BUFFSIZE);
                        if (!is_logged)
                        {
                            strcpy(sent_msg, "Welcome!");
                            is_logged = 1;
                        }
                        else
                        {
                            strcpy(sent_msg, "You are already logged in.");
                        }
                        break;
                        
                    }

                printf("[server]Trimitem mesajul... %s\n", sent_msg);
                if (write (client, sent_msg, strlen(sent_msg)) <= 0)
                {
                    perror ("[server]Eroare la write() catre client.\n");
                    continue;		/* continuam sa ascultam */
                }
                else
                    printf ("[server]Mesajul a fost trasmis cu succes.\n");
                
                bzero(sent_msg, BUFFSIZE);

                if (!strcmp(recv_msg, "quit"))
                {
                    close(client);
                    exit(0);
                }
                
            }
        exit(0);
        }

    }				/* while */
}				/* main */
