#include <unistd.h>
#include <stdint.h>
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
#define MAX_CONNECTIONS 50
#define BUFFER_SIZE (1024 * 32L)

#define ASCII_START 0 //32
#define ASCII_END 255 //126
#define ASCII_SET (ASCII_END - ASCII_START)

volatile sig_atomic_t running = 1;

// close gracefully
void sig_handler(int signo)
{
	printf("SIGNAL %d\n", signo);
	running = 0;
}

void signal_handler()
{
	struct sigaction signal_action;
	signal_action.sa_handler = sig_handler;
	sigemptyset(&signal_action.sa_mask);
	signal_action.sa_flags = 0;
	sigaction(SIGINT, &signal_action, NULL);

	struct sigaction
			oldact,
			act = {
					.sa_handler = SIG_IGN,
					.sa_flags = 0,
			};
	sigaction(SIGPIPE, &act, &oldact);
	// struct sigaction signal_ignore;
	// signal_ignore.sa_handler = SIG_IGN;
	// sigemptyset(&signal_ignore.sa_mask);
	// signal_ignore.sa_flags = 0;
	// sigaction(SIGPIPE, &signal_ignore, NULL);

	//sigaction(SIGPIPE, &new_action, NULL);
}

int pass(int result, char *msg_error)
{
	if (result == -1)
	{
		fprintf(stderr, "%s\n", msg_error);
		exit(1);
	}
	return result;
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
	write(connfd, &message->size, sizeof(u_int64_t));
	write(connfd, &message->hash, sizeof(hash_t));

	size_t buffer;
	size_t sent = 0;
	size_t total_sent = 0;

	while (message->size - total_sent)
	{
		buffer = (message->size - total_sent) > BUFFER_SIZE ? BUFFER_SIZE : (message->size - total_sent);
		sent = send(connfd, &message->bytes[total_sent], buffer, MSG_NOSIGNAL);
		total_sent += sent;
	}

	//	write(connfd, message->bytes, message->size);
}

// execute command
void process_request(int connfd, request_message_t *rm)
{
	time_t start, end;
	char buffer[25];
	char *bytes = gen_random_bytes(rm->size);

	message_t *message = create_message(bytes, rm->size);

	start = clock();
	reply_request(connfd, message);
	end = clock();

	printf("(%3d:%3d) SENT: %s bytes [%u hash] in %1fms\n",
				 connfd,
				 rm->serial,
				 bytes_to_human(message->size, buffer),
				 message->hash,
				 ELAPSED_MS);

	delete_message(message);
	delete_request_message(rm);
}

void *server(void *args)
{
	thread_args_t *thread_args = (thread_args_t *)args;
	int connfd = thread_args->fd;
	char buffer[25];

	request_message_t *request_message = malloc(sizeof(request_message_t));

	if (read(connfd, request_message, sizeof(request_message_t)))
	{

		// print received command
		printf("(%3d:%3d) REQU: %d - %s \n",
					 connfd,
					 request_message->serial,
					 request_message->size,
					 bytes_to_human(request_message->size, buffer));

		process_request(connfd, request_message);
	}

	delete_thread_args(thread_args);
	close(connfd);
	return NULL;
}

int establish_listen_socket(char *address, int port)
{
	int sockfd;
	struct sockaddr_in serv_addr;

	// create socket
	sockfd = pass(socket(AF_INET, SOCK_STREAM, 0), ERROR_SOCKET);

	// clear the structure
	bzero(&serv_addr, sizeof(serv_addr));

	// set the fields
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	inet_pton(AF_INET, address, &serv_addr.sin_addr);

	// bind the socket
	pass(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)), ERROR_BIND);

	// listen for connections
	pass(listen(sockfd, MAX_CONNECTIONS), ERROR_LISTEN);

	printf("Server started.\nSocket successfully binded at port %d.\n", port);

	return sockfd;
}

void wait_threads_end(pthread_t *threads, int n_threads)
{
	for (int i = 0; i < n_threads; i++)
	{
		pthread_join(threads[i], NULL);
	}
}

// Server
int main(int argc, char **argv)
{
	int opt;
	int sockfd, connfd;
	socklen_t len;
	int port = DEFAULT_PORT;
	struct sockaddr_in cli;
	char ipstr[INET_ADDRSTRLEN];
	char address[INET_ADDRSTRLEN];

	pthread_t thread[MAX_CONNECTIONS];
	int connections = 0;

	// get the port number from command line
	while ((opt = getopt(argc, argv, "p:s:")) != -1)
	{
		switch (opt)
		{
		case 'p':
			port = atoi(optarg);
			break;
		case 's':
			strcpy(address, optarg);
			break;
		default:
			if (opt == 's')
				fprintf(stderr, "Option -%c requires an argument.\n", opt);
			else if (isprint(opt))
				fprintf(stderr, "Unknown option `-%c'.\n", opt);
			exit(0);
		}
	}

	if (!*address)
	{
		printf("Usage: %s -s <server> [-p <port>]\n", argv[0]);
		exit(0);
	}

	setbuf(stdout, NULL);

	sockfd = establish_listen_socket(address, port);

	// set signal handler
	signal_handler();

	len = sizeof(cli);

	// Server loop
	while (running)
	{
		// Accept the data packet from client and verification
		connfd = accept(sockfd, (struct sockaddr *)&cli, &len);

		if (connfd < 0)
		{
			printf("Server accept failed...\n");
			break;
		}

		thread_args_t *args = create_thread_args(connfd, NULL);

		if (pthread_create(&thread[connections++], NULL, server, (void *)args) < 0)
		{
			perror("could not create thread");
			break;
		}

		strcpy(ipstr, inet_ntoa(cli.sin_addr));
		printf("[%3d:%3d] Connection accepted from %s\n", connections, connfd, ipstr);

		if (connections > MAX_CONNECTIONS)
		{
			printf("Maximum connections reached. (%d)\n", connections);

			wait_threads_end(thread, MAX_CONNECTIONS);
			connections = 0;
		}
	}

	wait_threads_end(thread, connections);

	printf("\nEnded.");
}
