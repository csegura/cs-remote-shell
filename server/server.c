#include <unistd.h>
#include <ctype.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <signal.h>
#include <time.h>

#define BUFFER_LEN 4096
#define DEFAULT_PORT 8080

// close gracefully
void sig_handler(int signo, const int sockfd)
{
	if (signo == SIGINT)
	{
		fprintf(stderr, "\n(@%d) Server Exiting.", getpid());
		close(sockfd);
		exit(0);
	}
}

// execute command and store output in buffer
char *execute_command(char *cmd)
{
	char *response = malloc(sizeof(char));
	char *chunk = calloc(BUFFER_LEN, sizeof(char));
	int len = 0;
	time_t start, end;
	start = clock();

	FILE *fp = popen(cmd, "r");

	if (fp == NULL)
	{
		fprintf(stderr, "\nError opening file");
		exit(1);
	}

	while (fgets(chunk, sizeof(char) * BUFFER_LEN, fp) != 0)
	{
		int chunk_len = strlen(chunk);
		response = realloc(response, strlen(response) + chunk_len);
		memcpy(&response[len], chunk, chunk_len);
		len += chunk_len;
		bzero(chunk, BUFFER_LEN);
	}

	end = clock();

	// command that not have output
	// response 1 byte
	if (len == 0)
		strcpy(response, " ");

	printf("RESP: %1fms - %lu bytes\n%s",
				 (double)(end - start) / CLOCKS_PER_SEC / 1000,
				 strlen(response),
				 response);

	pclose(fp);

	return response;
}

// Server operation is a remote shell
// commands are executed on this server
// and stdout is sent back to client
void remote_shell(int sockfd, pid_t pid)
{
	char cmd[BUFFER_LEN];
	char *response;

	while (1)
	{
		bzero(cmd, BUFFER_LEN);

		// read the command from client and copy it in buffer
		if (read(sockfd, cmd, sizeof(cmd)))
		{
			// exit command close server
			if (strncmp("exit", cmd, 4) == 0)
			{
				printf("(%d) Conexon closed.\n", pid);
				break;
			}

			// print buffer which contains the client contents
			printf("\n(@%d) RCMD: %s", pid, cmd);

			response = execute_command(cmd);
			write(sockfd, response, strlen(response));
			free(response);
		}
	}
}

// Server
int main(int argc, char **argv)
{
	int opt;
	int sockfd, connfd;
	int port = DEFAULT_PORT;
	pid_t up_pid;

	struct sockaddr_in servaddr, cli;
	char ipstr[INET_ADDRSTRLEN];

	// get the port number from command line
	while ((opt = getopt(argc, argv, "p:")) != -1)
	{
		switch (opt)
		{
		case 'p':
			port = atoi(optarg);
			break;
		}
	}

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1)
	{
		printf("Socket creation failed...\n");
		exit(1);
	}
	else
		printf("Socket successfully created..\n");

	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(port);

	// binding newly created socket to given IP and verification
	if ((bind(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr))) != 0)
	{
		printf("Socket bind failed in port %d.\n", port);
		exit(1);
	}
	else
		printf("Socket successfully binded at port %d.\n", port);

	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0)
	{
		printf("Listen failed...\n");
		exit(1);
	}
	else
		printf("Server listening..\n");

	signal(SIGINT, (void *)sig_handler);

	socklen_t len = sizeof(cli);

	while (1)
	{
		// accept the data packet from client
		connfd = accept(sockfd, (struct sockaddr *)&cli, &len);

		if (connfd < 0)
		{
			printf("Server accept failed...\n");
			exit(1);
		}
		else
		{
			// show connection
			strcpy(ipstr, inet_ntoa(cli.sin_addr));
			printf("(@%d) Connection accepted from %s\n", getpid(), ipstr);
		}

		// fork - Child handles this connection, parent listens for another
		up_pid = fork();

		if (up_pid == -1)
		{
			perror("fork");
			exit(1);
		}

		if (up_pid == 0)
		{
			remote_shell(connfd, getpid());
		}
	}

	// close the socket - sure SIGINT will handle this
	if (sockfd)
		close(sockfd);
}
