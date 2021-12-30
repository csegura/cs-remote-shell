#include <unistd.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <limits.h>
#include <time.h>
#include "../b-server/b-funcs.h"

#define COMMAND_LEN 128
#define BUFFER_LEN 4096
#define DEFAULT_PORT 8080
#define DEFAULT_SERVER "127.0.0.1"

char *get_command()
{
	char *cmd = calloc(COMMAND_LEN, sizeof(char));
	int n = 0;
	printf("\nCMD: ");
	while ((cmd[n++] = getchar()) != '\n')
		;
	return cmd;
}

message_t *get_message(int sockfd)
{
	message_t *message = create_message(NULL, 0);
	char *chunk = calloc(BUFFER_LEN, sizeof(char));

	while (1)
	{
		int size = read(sockfd, chunk, BUFFER_LEN);
		if (size == 0)
			break;
		message->bytes = realloc(message->bytes, message->size + size);
		memcpy(&message->bytes[message->size], chunk, size);
		message->size += size;
		if (size < BUFFER_LEN)
			break;
		bzero(chunk, BUFFER_LEN);
	}

	return message;
}

void func(int sockfd)
{
	char *cmd;
	message_t *message;
	time_t start, end;

	while (1)
	{
		cmd = get_command();
		write(sockfd, cmd, strlen(cmd));

		if ((strncmp(cmd, "exit", 4)) == 0)
		{
			printf("Client Exit...\n");
			break;
		}
		start = clock();
		message = get_message(sockfd);
		end = clock();

		unsigned int received_hash = get_hash(message);
		printf("RCVD: %d bytes - Message %d bytes [%u hash] in %1fms \n",
					 message->size,
					 message->size - HASH_SIZE,
					 received_hash,
					 (double)(end - start) / CLOCKS_PER_SEC / 1000);

		unsigned int hash = calc_hash(message, 1);
		printf("\nRESP: hash %u -> calculated %u\n", received_hash, hash);

		free(message->bytes);
		free(message);
	}
}

int main(int argc, char **argv)
{
	int opt;
	int port = DEFAULT_PORT;
	char *server = DEFAULT_SERVER;

	while ((opt = getopt(argc, argv, "s:p:")) != -1)
	{
		switch (opt)
		{
		case 's':
			server = optarg;
			break;
		case 'p':
			port = atoi(optarg);
			break;
		default:
			if (opt == 's')
				fprintf(stderr, "Option -%c requires an argument.\n", opt);
			else if (isprint(opt))
				fprintf(stderr, "Unknown option `-%c'.\n", opt);
			exit(0);
		}
	}

	if (!server)
	{
		printf("Usage: %s -s <server> [-p <port>]\n", argv[0]);
		exit(0);
	}

	printf("Connecting to %s:%d\n", server, port);

	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;

	// socket create and varification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		printf("Socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");

	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(server);
	servaddr.sin_port = htons(port);

	// connect the client socket to server socket
	if (connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0)
	{
		printf("Connection failed.\n");
		exit(0);
	}
	else
		printf("Connected.\n");

	// function for chat
	func(sockfd);

	// close the socket
	close(sockfd);
}
