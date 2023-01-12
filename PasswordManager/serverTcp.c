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

void delete_record(char *record, char *filename)
{
    FILE *fp, *ft;
    char line[50];

    ft = fopen("temp.txt", "w");
    fp = fopen(filename, "r");
    while (fgets(line, 50, fp) != NULL)
    {
        if (!strstr(line, record))
            fputs(line, ft);
    }

    fclose(fp);
    fclose(ft);
    remove(filename);
    rename("temp.txt", filename);
}

void show_ctg_pass(char *str, char *ctg, char **ctgpass, char *filename)
{
    FILE *fp;
    char line[50];
    char info[BUFFSIZE];

    fp = fopen(filename, "r");
    if (!fp)
    {
            perror("File does not exist!");
            exit(0);
    }
    strcpy(info, "");
    while (fgets(line, 50, fp) != NULL)
    {
        if (strstr(line, ctg))
        {
            strcat(info, line);
        }
    }
    *ctgpass = info;
    fclose(fp);
}

void show_pass(char *str, char **allpass, char *filename)
{
    FILE *fp;
    char line[50];
    char info[BUFFSIZE];

    fp = fopen(filename, "r");
    if (!fp)
    {
            perror("File does not exist!");
            exit(0);
    }
    strcpy(info, "");
    while (fgets(line, 50, fp) != NULL)
    {
        strcat(info, line);
    }
    *allpass = info;
    fclose(fp);
}

int get_details(char *str, char **usn, char **pws, char **ctg, char **ttl, char **nts, char **url)
{
    char delim[] = " ";

	char *ptr = strtok(str, delim);
    int no = 0;

	while(ptr != NULL)
	{
		printf("'%s'\n", ptr);
        if (no == 1){
            *usn = ptr;
        }
        else if (no == 2){
            *pws = ptr;
        }
        else if (no == 3){
            *ctg = ptr;
        }
        else if (no == 4){
            *ttl = ptr;
        }
        else if (no == 5){
            *nts = ptr;
        }
        else if (no == 6){
            *url = ptr;
        }
        else
        {
            *usn = " ";
            *pws = " ";
            *ctg = " ";
            *ttl = " ";
            *nts = " ";
            *url = " ";
        }
        no++;
        // printf(" get details no = %u\n", no);
		ptr = strtok(NULL, delim);
	}
    if (no < 4)
    {
        return 1;
    }
    return 0;
}

int add_record(char *record, char *filename)
{
    FILE *fp;
    char line[50];
    // printf("filename= %s\n", filename);

    fp = fopen(filename, "r");
    if (!fp)
    {
        printf("%s\n", "File does not exist...creating now.");
        // perror("File does not exist!");
        fp = fopen(filename, "w");
        // exit(0);
    }
    fclose(fp);
    fp = fopen(filename, "a");
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
    if (strcmp(s, "show") == 0)
		return 3;
    if (strstr(s, "show"))
		return 4;
    if (!strcmp(s, "delete"))
		return 0;
    if (strstr(s, "delete"))
		return 5;
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
            char logged_user[20];
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
                // printf("cod = %u\n", ret_cod(recv_msg));
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
                            strcpy(sent_msg, "Syntax: login {username} {password}");
                        }
                        else{
                            // printf("usn: %s\npass: %s\n", username, password);
                            char record[50];
                            strcpy(record, username);
                            strcat(record, ";");
                            strcat(record, password);
                            if (check_credentials(record))
                            {
                                logged_in = 1;
                                strcpy(logged_user, username);
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
                            strcpy(sent_msg, "Syntax: register {username} {password}");
                        }
                        else
                        {
                            // printf("new usn: %s\nnew pass: %s\n", newUsername, newPassword);
                            char record[50];
                            strcpy(record, "\n");
                            strcat(record, newUsername);
                            strcat(record, ";");
                            strcat(record, newPassword);
                            strcat(record, ";");
                            if(add_record(record, "users.txt"))
                            {
                                strcpy(sent_msg, "New record registered.");
                            }
                            else
                            {
                                strcpy(sent_msg, "Something wrong at registering.");
                            }
                        }
                    }
                    else{
                    
                        char *usn, *pws, *ctg, *ttl, *nts, *url;

                        if (get_details(recv_msg, &usn, &pws, &ctg, &ttl, &nts, &url))
                        {
                            strcpy(sent_msg, "Syntax: register {username} {password} {category} [{title, notes, url}]");

                        }
                        else
                        {
                            // printf("%s, %s, %s, %s, %s, %s\n", usn, pws, ctg, ttl, nts, url);
                            char new_record[300];
                            char filename[20];
                            strcpy(new_record, "\n");
                            strcat(new_record, pws);
                            strcat(new_record, ";");
                            strcat(new_record, usn);
                            strcat(new_record, ";");
                            strcat(new_record, ctg);
                            strcat(new_record, ";");
                            strcat(new_record, ttl);
                            strcat(new_record, ";");
                            strcat(new_record, nts);
                            strcat(new_record, ";");
                            strcat(new_record, url);
                            strcat(new_record, ";");

                            strcpy(filename, logged_user);
                            strcat(filename, "manager.txt");
                            // printf("record= %s\nfilename= %s\n", new_record, filename);
                            
                            if(add_record(new_record, filename))
                            {
                                strcpy(sent_msg, "Registered.");
                            }
                            else
                            {
                                strcpy(sent_msg, "Something wrong at registering.");
                            }
                        }

                    }
                    break;
                case 3:
                    bzero(sent_msg, BUFFSIZE);
                    if(!logged_in)
                    {
					    strcpy(sent_msg, "You have to log in to show passwords.");
                    }
                    else
                    {
                        char *allpass;
                        char filename[50];
                        strcpy(filename, logged_user);
                        strcat(filename, "manager.txt");
                        show_pass(recv_msg, &allpass, filename);
					    strcpy(sent_msg, allpass);
                    }

                    break;
                case 4:
                    bzero(sent_msg, BUFFSIZE);
                    if(!logged_in)
                    {
					    strcpy(sent_msg, "You have to log in to show passwords.");
                    }
                    else
                    {
                        char category[10];
                        char delim[] = " ";
                        char *ptr = strtok(recv_msg, delim);
                        int no = 0;
                        while(ptr != NULL)
                        {
                            strcpy(category, ptr);
                            ptr = strtok(NULL, delim);
                        }
                        char *ctgpass;
                        char filename[50];
                        strcpy(filename, logged_user);
                        strcat(filename, "manager.txt");
                        show_ctg_pass(recv_msg, category, &ctgpass, filename);
					    strcpy(sent_msg, ctgpass);
                    }
                
                    break;
                case 5:
                    bzero(sent_msg, BUFFSIZE);
                    if(!logged_in)
                    {
					    strcpy(sent_msg, "You have to log in to delete records.");
                    }
                    else
                    {
                        char usn[30], psw[30], filename[30], record[60];
                        char delim[] = " ";
                        char *ptr = strtok(recv_msg, delim);
                        int no = 0;
                        while(ptr != NULL)
                        {
                            if(no == 1)
                            {
                                strcpy(usn, ptr);
                            }
                            else if(no == 2)
                            {
                                strcpy(psw, ptr);
                            }
                            no++;
                            ptr = strtok(NULL, delim);
                        }
                        // printf("delete usn = %s and %s\n", usn, psw);
                        strcpy(filename, logged_user);
                        strcat(filename, "manager.txt");
                        strcpy(record, psw);
                        strcat(record, ";");
                        strcat(record, usn);
                        // printf("delete record %s\n", record);
                        delete_record(record, filename);
					    strcpy(sent_msg, "Record deleted. Type show to verify.");
                    }

                    break;
                default:
					bzero(sent_msg, BUFFSIZE);
					strcpy(sent_msg, "Command not valid.");
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
