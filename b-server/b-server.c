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
#include <pthread.h>
#include "b-funcs.h"

#define COMMAND_LEN 24
#define DEFAULT_PORT 8080
#define MAX_CONNECTIONS 50

#define ASCII_START 0 //32
#define ASCII_END 255 //126
#define ASCII_SET (ASCII_END - ASCII_START)

// close gracefully
void sig_handler(int signo)
{
	if (signo == SIGINT)
	{
		printf("SIGINT\n");
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

void reply_request(int connfd, message_t *message)
{
	write(connfd, &message->size, sizeof(size_t));
	write(connfd, &message->hash, sizeof(hash_t));
	write(connfd, message->bytes, message->size);
	close(connfd);
}

// execute command
void process_request(int connfd, request_message_t *rm)
{
	time_t start, end;

	char *bytes = gen_random_bytes(rm->size);

	message_t *message = create_message(bytes, rm->size);

	start = clock();
	reply_request(connfd, message);
	end = clock();

	printf("(%3d:%3d) SENT: %s bytes [%u hash] in %1fms\n",
				 connfd,
				 rm->serial,
				 bytes_to_human(message->size),
				 message->hash,
				 (double)(end - start) / CLOCKS_PER_SEC / 1000);

	delete_message(message);
}

void *server(void *args)
{
	thread_args_t *thread_args = (thread_args_t *)args;
	int connfd = thread_args->fd;

	request_message_t *request_message = malloc(sizeof(request_message_t));

	while (1)
	{
		if (read(connfd, request_message, sizeof(request_message_t)))
		{
			//decode_request_message(request_message);

			// print received command
			printf("(%3d:%3d) REQU: %d - %s \n",
						 connfd,
						 request_message->serial,
						 request_message->size,
						 bytes_to_human(request_message->size));

			process_request(connfd, request_message);
			break;
		}
	}

	delete_thread_args(args);
	close(connfd);
	return NULL;
}

int establish_listen_socket(int port)
{
	int sockfd;
	struct sockaddr_in serv_addr;

	// create socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("Socket creation error.\n");
		exit(1);
	}

	// clear the structure
	bzero(&serv_addr, sizeof(serv_addr));

	// set the fields
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);

	// bind the socket
	if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		printf("Bind error\n");
		printf("Port: %d could be in use\n", port);
		exit(1);
	}

	// listen for connections
	if (listen(sockfd, MAX_CONNECTIONS) < 0)
	{
		printf("Listen error\n");
		exit(1);
	}

	printf("\nServer started.\nSocket successfully binded at port %d.\n", port);

	return sockfd;
}

// Server
int main(int argc, char **argv)
{
	int opt;
	int sockfd, connfd, len;
	int port = DEFAULT_PORT;
	struct sockaddr_in servaddr, cli;
	char ipstr[INET_ADDRSTRLEN];

	pthread_t thread[MAX_CONNECTIONS];
	int connections = 0;

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

	setbuf(stdout, NULL);

	sockfd = establish_listen_socket(port);

	// set signal handler
	signal(SIGINT, (void *)sig_handler);

	len = sizeof(cli);

	// Server loop
	while (1)
	{
		// Accept the data packet from client and verification
		connfd = accept(sockfd, (struct sockaddr *)&cli, &len);

		if (connfd < 0)
		{
			printf("Server accept failed...\n");
			exit(1);
		}

		thread_args_t *args = create_thread_args(connfd, NULL);

		if (pthread_create(&thread[connections++], NULL, server, (void *)args) < 0)
		{
			perror("could not create thread");
			exit(1);
		}

		int res = getpeername(connfd, (struct sockaddr *)&cli, &len);
		strcpy(ipstr, inet_ntoa(cli.sin_addr));
		printf("[%3d:%3d] Connection accepted from %s\n", connections, connfd, ipstr);

		if (connections >= MAX_CONNECTIONS)
		{
			printf("Maximum connections reached. (%d)\n", connections);

			for (connections = 0; connections < MAX_CONNECTIONS; connections++)
			{
				pthread_join(thread[connections], NULL);
			}

			printf("Finished. (%d)\n", connections);

			connections = 0;
		}
	}
	printf("\nEnded.");
}
