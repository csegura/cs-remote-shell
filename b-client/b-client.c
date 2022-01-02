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

#define MAX_CONNECTIONS 50

message_t *get_message(request_t *request)
{

	size_t size;
	read(request->fd, &size, sizeof(size_t));

	hash_t hash;
	read(request->fd, &hash, sizeof(hash_value));

	message_t *message = create_message(NULL, 0);
	size_t buffer_size = size > 65535 ? 65535 : size;
	char *chunk = (char *)calloc(buffer_size, sizeof(char));

	while (1)
	{
		int received = read(request->fd, chunk, buffer_size);
		if (received == 0)
			break;

		message->bytes = (char *)realloc(message->bytes, message->size + received);
		memcpy(&message->bytes[message->size], chunk, received);
		message->size += received;

		if (size < buffer_size)
			break;
	}

	message->hash = calc_hash(message);

	if (hash != message->hash)
	{
		message->hash = 0;
	}

	free(chunk);
	return message;
}

void *client(void *args)
{
	request_t *request = (request_t *)args;
	message_t *message;
	time_t start, end;
	char buffer[25];
	int *error = (int *)malloc(sizeof(int));

	request_message_t *request_message = create_request_message(request);
	write(request->fd, request_message, sizeof(request_message_t));

	start = clock();
	message = get_message(request);
	end = clock();

	printf("[%2d:%3d] %s RCVD: Message %s bytes in %3fms \n",
				 request->fd,
				 request->serial,
				 message->hash == 0 ? "FAIL" : "-OK-",
				 bytes_to_human(message->size, buffer),
				 (double)(end - start) / (CLOCKS_PER_SEC / 1000));

	*error = message->hash == 0 ? 1 : 0;

	delete_message(message);
	close(request->fd);
	delete_request(request);
	delete_request_message(request_message);

	pthread_exit(error);
}

int connect_server(char *server, int port)
{

	int sockfd, connfd;
	struct sockaddr_in servaddr;

	// socket create and varification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	if (sockfd == -1)
	{
		printf("Socket creation failed...\n");
		exit(0);
	}

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

	return sockfd;
}

int wait_threads_end(pthread_t *threads, int n_threads)
{
	int errors = 0;
	int *error;
	int i;

	for (i = 0; i < n_threads; i++)
	{
		pthread_join(threads[i], (void *)&error);
		errors += *error;
		free(error);
	}
	return errors;
}

int main(int argc, char **argv)
{
	int opt;
	int port = DEFAULT_PORT;
	char *server = DEFAULT_SERVER;
	int n_requests = 0;
	char *command = NULL;
	pthread_t threads[MAX_CONNECTIONS];
	int n_threads = 0;
	time_t start, end;

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
			n_requests = atoi(optarg);
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
		printf("Usage: %s -s <server> [-p <port>] -c <size_in_kb> -f <n_requests>\n", argv[0]);
		exit(0);
	}

	printf("n_requests: %d\n", n_requests);

	int errors = 0;
	size_t size = atoi(command) * 1024;
	time(&start);

	for (int i = 0; i < n_requests; i++)
	{
		int connfd = connect_server(server, port);

		request_t *request = create_request(connfd, i, size, 0);

		if (pthread_create(&threads[n_threads++], NULL, client, (void *)request) < 0)
		{
			perror("could not create thread");
			exit(1);
		}

		if (n_threads > MAX_CONNECTIONS)
		{
			printf("Maximum connections reached. (%d)\n", MAX_CONNECTIONS);
			errors += wait_threads_end(threads, MAX_CONNECTIONS);
			n_threads = 0;
		}
	}

	errors += wait_threads_end(threads, n_threads);

	time(&end);

	char buffer[25];
	printf("Elapsed time %2fs - %d errors - %s reived\n",
				 (double)(end - start),
				 errors,
				 bytes_to_human(size * n_requests, buffer));

	return 0;
}
