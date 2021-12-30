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
#include <limits.h>
#include "b-funcs.h"

#define COMMAND_LEN 24
#define DEFAULT_PORT 8080

#define ASCII_START 32
#define ASCII_END 126
#define ASCII_SET (ASCII_END - ASCII_START)

// close gracefully
void sig_handler(int signo, const int sockfd)
{
	if (signo == SIGINT)
	{
		fprintf(stderr, "\n(@%d) Exiting.", getpid());
		close(sockfd);
		exit(0);
	}
}

char *gen_random_bytes(int size)
{
	char *bytes = malloc(size);
	for (int i = 0; i < size; i++)
	{
		bytes[i] = (char)(rand() % ASCII_SET) + ASCII_START;
	}
	return bytes;
}

ssize_t send_message(int sockfd, message_t *message)
{
	ssize_t sent = write(sockfd, message->bytes, message->size);
	sent += write(sockfd, &message->hash.encoded, HASH_SIZE);
	return sent;
}

// execute command
void execute_command(int sockfd, char *cmd)
{
	time_t start, end;

	char *param[10];
	int i = 0;

	for (char *p = strtok(cmd, " "); p != NULL; p = strtok(NULL, " "))
	{
		param[i++] = p;
	}

	// param[0] = size in kb

	int size = atoi(param[0]) * 1024;

	char *bytes = gen_random_bytes(size);
	message_t *message = create_message(bytes, size);

	start = clock();

	ssize_t send = send_message(sockfd, message);

	end = clock();

	printf("SENT: %d bytes - Message %d bytes [%u hash] in %1fms \n",
				 send,
				 message->size,
				 message->hash.value,
				 (double)(end - start) / CLOCKS_PER_SEC / 1000);
}

void server(int sockfd, pid_t pid)
{
	char cmd[COMMAND_LEN];

	while (1)
	{
		bzero(cmd, COMMAND_LEN);

		// read the command from client and copy it in buffer
		if (read(sockfd, cmd, sizeof(cmd)))
		{
			// exit command close server
			if (strncmp("exit", cmd, 4) == 0)
			{
				printf("(@%d) Conexon closed.\n", pid);
				break;
			}

			// print received command
			printf("\n(@%d) RCMD: %s", pid, cmd);

			execute_command(sockfd, cmd);
		}
	}
}

// Server
int main(int argc, char **argv)
{
	int opt;
	int sockfd, connfd, len;
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

	len = sizeof(cli);

	while (1)
	{
		// Accept the data packet from client and verification
		connfd = accept(sockfd, (struct sockaddr *)&cli, &len);

		if (connfd < 0)
		{
			printf("Server accept failed...\n");
			exit(1);
		}
		else
		{
			int res = getpeername(connfd, (struct sockaddr *)&cli, &len);
			strcpy(ipstr, inet_ntoa(cli.sin_addr));
			printf("(@%d) Connection accepted from %s\n", getpid(), ipstr);
		}

		// Fork - Child handles this connection, parent listens for another
		up_pid = fork();

		if (up_pid == -1)
		{
			perror("fork");
			exit(1);
		}

		if (up_pid == 0)
		{
			server(connfd, getpid());
		}
	}

	// close the socket - sure SIGINT will handle this
	if (sockfd)
		close(sockfd);
}
