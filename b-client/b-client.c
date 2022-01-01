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
#include <pthread.h>
#include "../b-server/b-funcs.h"

#define COMMAND_LEN 128
#define BUFFER_LEN 4096
#define MAX_CONNECTIONS 1000

char *get_command()
{
	char *cmd = calloc(COMMAND_LEN, sizeof(char));
	int n = 0;
	printf("\nCMD: ");
	while ((cmd[n++] = getchar()) != '\n')
		;
	return cmd;
}

// void check_message(message_t *message)
// {
// 	hash_value received_hash = get_hash(message);
// 	hash_value calculated_hash = calc_hash(message, 1);

// 	if (received_hash != calculated_hash)
// 	{
// 		printf("ERROR: hash mismatch - Received %d Calc %d\n",
// 					 received_hash,
// 					 calculated_hash);
// 	}
// 	else
// 	{
// 		message->hash.value = received_hash;
// 		// todo: adjust message->size
// 	}
// }

message_t *get_message(request_t *request)
{

	size_t size;
	read(request->fd, &size, sizeof(size_t));
	printf("size: %d\n", size);

	hash_t hash;
	read(request->fd, &hash, sizeof(hash_value));
	printf("hash: %d\n", hash);

	message_t *message = create_message(NULL, 0);
	size_t buffer_size = size > 65535 ? 65535 : size;
	char *chunk = calloc(buffer_size, sizeof(char));

	while (1)
	{
		int received = read(request->fd, chunk, buffer_size);
		if (received == 0)
			break;
		message->bytes = realloc(message->bytes, message->size + received);
		memcpy(&message->bytes[message->size], chunk, received);
		message->size += received;
		if (size < buffer_size)
			break;
		bzero(chunk, buffer_size);
	}

	printf("RCVD bytes: %d - %d\n", message->size, size);
	message->hash = calc_hash(message);

	if (hash != message->hash)
	{

		printf("ERROR: hash mismatch - Received %d Calc %d\n",
					 hash,
					 message->hash);
	}

	return message;
}

void *client(void *args)
{
	request_t *request = (request_t *)args;

	message_t *message;
	time_t start, end;

	// printf("[%2d:%3d] SENT: %s cmd\n",
	// 			 request->fd,
	// 			 request->serial,
	// 			 bytes_to_human(request->size));

	request_message_t *request_message = create_request_message(request);
	write(request->fd, request_message, sizeof(request_message_t));

	start = clock();
	message = get_message(request);
	end = clock();

	printf("[%2d:%3d] %s RCVD: Message %s bytes in %1fms \n",
				 request->fd,
				 request->serial,
				 message->hash == 0 ? "FAIL" : "-OK-",
				 bytes_to_human(message->size),
				 (double)(end - start) / CLOCKS_PER_SEC / 1000);

	delete_message(message);
	close(request->fd);
	delete_request(request);
	delete_request_message(request_message);
}

int connect_server(char *server, int port)
{

	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;

	printf("Connecting to %s:%d\n", server, port);

	// socket create and varification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1)
	{
		printf("Socket creation failed...\n");
		exit(0);
	}
	//else
	//printf("Socket successfully created..\n");

	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(server);
	servaddr.sin_port = htons(port);

	// connect the client socket to server socket
	connfd = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	if (connfd != 0)
	{
		printf("Connection failed.\n");
		exit(0);
	}
	else
		printf("Connected.\n");

	return sockfd;
}

int main(int argc, char **argv)
{
	int opt;
	int port = DEFAULT_PORT;
	char *server = DEFAULT_SERVER;
	int threads = 1;
	char *command = NULL;
	pthread_t thread[MAX_CONNECTIONS];
	int connections = 0;

	while ((opt = getopt(argc, argv, "s:p:f:c:")) != -1)
	{
		switch (opt)
		{
		case 's':
			server = optarg;
			break;
		case 'p':
			port = atoi(optarg);
			break;
		case 'f':
			threads = atoi(optarg);
			break;
		case 'c':
			command = optarg;
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

	printf("\ncommand: %s | proccess %d\n", command, threads);

	for (int i = 0; i < threads; i++)
	{
		int connfd = connect_server(server, port);

		size_t size = atoi(command) * 1024;

		printf("%lu %s\n", size, bytes_to_human(size));

		request_t *request = create_request(connfd, i, size, 0);

		if (pthread_create(&thread[connections++], NULL, client, (void *)request) < 0)
		{
			perror("could not create thread");
			exit(1);
		}

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

	// wait for all threads to finish
	for (int i = 0; i < threads; i++)
		pthread_join(thread[i], NULL);

	return 0;
}
