#include <unistd.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define COMMAND_LEN 128
#define BUFFER_LEN 4096
#define DEFAULT_PORT 8080
#define DEFAULT_SERVER "127.0.0.1"

char *get_command()
{
	char *cmd = malloc(COMMAND_LEN);
	int n = 0;
	printf("\nCMD: ");
	while ((cmd[n++] = getchar()) != '\n')
		;
	return cmd;
}

char *get_response(int sockfd)
{
	char *chunk = malloc(BUFFER_LEN);
	char *response = NULL;
	int len = 0;

	while (1)
	{
		bzero(chunk, BUFFER_LEN);
		int response_len = read(sockfd, chunk, BUFFER_LEN);
		if (response_len == 0)
			break;
		response = realloc(response, len + response_len);
		memcpy(&response[len], chunk, response_len);
		len += response_len;
		if (response_len < BUFFER_LEN)
			break;
	}

	free(chunk);
	return response;
}

void client(int sockfd)
{
	char *cmd;
	char *response;

	while (1)
	{
		cmd = get_command();

		if ((strncmp(cmd, "exit", 4)) == 0)
		{
			printf("Client Exit...\n");
			free(cmd);
			break;
		}
		else
		{

			write(sockfd, cmd, strlen(cmd));

			response = get_response(sockfd);
			printf("\nRESP: %lu bytes\n%s", strlen(response), response);

			free(cmd);
			free(response);
		}
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

	int sockfd;
	struct sockaddr_in servaddr;

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

	// client
	client(sockfd);

	// close the socket
	close(sockfd);
}
