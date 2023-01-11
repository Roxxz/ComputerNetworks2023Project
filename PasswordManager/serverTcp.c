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

int add_record(char *record)
{
    FILE *fp;
    char filename[] = "users.txt";
    char line[50];

    fp = fopen(filename, "a");
    if (!fp)
    {
        perror("File does not exist!");
        exit(0);
    }
    printf("record= %s\n", record);
    fputs(record, fp);
    fclose(fp);
    return 1;
}

int check_credentials(char *record)
{
    FILE *fp;
    char filename[] = "users.txt";
    char line[50];

    fp = fopen(filename, "r");
    if (!fp)
    {
            perror("File does not exist!");
            exit(0);
    }
    while (fgets(line, 50, fp) != NULL)
    {
        line[strcspn(line, "\n")] = 0;
        if (strstr(line, record))
        {
                fclose(fp);
                return 1;
        }
    }
    fclose(fp);
    return 0;
}

int get_credentials(char *str, char **username, char **password)
{
	char delim[] = " ";

	char *ptr = strtok(str, delim);
    int no = 0;

	while(ptr != NULL)
	{
		// printf("'%s'\n", ptr);
        if (no == 1){
            *username = ptr;
        }
        else if (no == 2){
            *password = ptr;
        }
        no++;
		ptr = strtok(NULL, delim);
	}
    if (no <= 2)
        {
            return 1;
        }
    return 0;
}

int ret_cod(char *s)
{
	if (!strcmp(s, "quit"))
		return -1;
	if (strstr(s, "login"))
		return 1;
    if (strstr(s, "register"))
		return 2;
	return 0;
}

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
		int length = sizeof(from);
        printf ("[server]Asteptam la portul %d...\n",PORT);
    	fflush (stdout);

		client = accept(sd, (struct sockaddr *)&from, &length);

		if (client < 0)
		{
			perror("Eroare la accept().\n");
			continue;
		}

		int pid;
		if ((pid = fork()) == -1)
		{
			close(client);
			continue;
		}
		else if (pid > 0)
		{
			close(client);
			while (waitpid(-1, NULL, WNOHANG));
			continue;
		}
		else if (pid == 0)
		{
			// copil, pentru fiecare client
			int logged_in = 0;
			close(sd);
			if (write(client, welcome_msg, strlen(welcome_msg)) <= 0)
			{
				perror("Eroare la scriere catre client.\n");
				exit(0); /* continuam sa ascultam */
			}
			else
				printf("Mesajul welcome fost trasmis cu succes.\n");

			while (1)
			{
				printf("Asteptam mesajul...\n");
				bzero(recv_msg, 300);
				if (read(client, recv_msg, 300) <= 0)
				{
					perror("Eroare la citire de la client.\n");
					close(client);
					break;
				}
				printf("Mesajul primit: %s\n", recv_msg);

				// int cod_msg = ret_cod(recv_msg);
                printf("cod = %u\n", ret_cod(recv_msg));
				switch (ret_cod(recv_msg))
				{
				case -1:
					bzero(sent_msg, BUFFSIZE);
					strcpy(sent_msg, "Exiting");
					break;
				case 1:
					bzero(sent_msg, BUFFSIZE);
					if (!logged_in)
					{
                        //separate username and password
                        char *username, *password;
                        if (get_credentials(recv_msg, &username, &password))
                        {
                            strcpy(sent_msg, "Syntax is: login {username} {password}");
                        }
                        else{
                            printf("usn: %s\npass: %s\n", username, password);
                            char record[50];
                            strcpy(record, username);
                            strcpy(record, ";");
                            strcpy(record, password);
                            if (check_credentials(record))
                            {
                                logged_in = 1;
                                strcpy(sent_msg, "You are logged now.");
                            }
                            else
                            {
                                strcpy(sent_msg, "No such record. Try again or register.");
                            }
                        }
					}
					else
					{
						strcpy(sent_msg, "!!! You are already logged in !!!");
					}

					break;
                case 2:
                    bzero(sent_msg, BUFFSIZE);
                    if (!logged_in)
                    {
                        char *newUsername, *newPassword;
                        if (get_credentials(recv_msg, &newUsername, &newPassword))
                        {
                            strcpy(sent_msg, "Syntax is: register {username} {password}");
                        }
                        else
                        {
                            printf("new usn: %s\nnew pass: %s\n", newUsername, newPassword);
                            char record[50];
                            // strcpy(record, newUsername);
                            // printf("record= %s\n", record);
                            strcpy(newUsername, ";");
                            printf("record= %s\n", newUsername);
                            strcpy(newUsername, newPassword);
                            strcpy(newUsername, ";");
                            printf("record= %s\n", newUsername);
                            strcpy(sent_msg, "testing.");
                            // if(add_record(record))
                            // {
                            //     strcpy(sent_msg, "New record registered.");
                            // }
                            // else
                            // {
                            //     strcpy(sent_msg, "Something wrong at registering.");
                            // }
                        }
                    }
                    else{
                        char *newUsername, *newPassword;
                        get_credentials(recv_msg, &newUsername, &newPassword);
                        printf("new usn: %s\nnew pass: %s\n", newUsername, newPassword);
                        strcpy(sent_msg, "Registered.");
                    }
                    break;
                default:
					bzero(sent_msg, BUFFSIZE);
					strcpy(sent_msg, "Command not valid");
					break;
				}
				printf("Trimitem mesajul:%s\n", sent_msg);
				if (write(client, sent_msg, strlen(sent_msg)) <= 0)
				{
					perror("Eroare la scriere catre client.\n");
					continue;
				}
				else
					printf("Mesaj trimis\n");
				bzero(sent_msg, BUFFSIZE);
				if (ret_cod(recv_msg) == -1)
				{
					close(client);
					exit(0);
				}
			}
			exit(0);
		}
	}

}				/* main */
