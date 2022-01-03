#ifndef B_FUNCS_H
#define B_FUNCS_H
#include <stdio.h>
#include <stdint.h>

#define ELAPSED_MS	((double)(end - start) / (CLOCKS_PER_SEC/1000))
#define ERROR_SOCKET "Socket error"
#define ERROR_BIND "Bind error (port could be in use)"
#define ERROR_LISTEN "Listen error"

#define DEFAULT_PORT 8080
#define DEFAULT_SERVER "127.0.0.1"

typedef unsigned int hash_value;

#define HASH_SIZE sizeof(hash_value)

typedef struct
{
	int fd;
	int thread;
	void *other;
} thread_args_t;

typedef unsigned int hash_t;

typedef struct
{
	int serial;
	char pad[4];
	size_t size;
	int type;
} request_message_t;

typedef struct
{
	int fd;
	int serial;
	size_t size;
	int type;
} request_t;

typedef struct
{
	char *bytes;
	uint64_t size;
	hash_t hash;
} message_t;

hash_t calc_hash(message_t *message);
//hash_value get_hash(message_t *message);

message_t *create_message(char *bytes, size_t size);
void delete_message(message_t *message);

request_t *create_request(int fd, int serial, size_t size, int type);
void delete_request(request_t *request);
request_message_t *create_request_message(request_t *request);
void decode_request_message(request_message_t *rm);
void delete_request_message(request_message_t *rm);

thread_args_t *create_thread_args(int fd, char *cmd);
void delete_thread_args(thread_args_t *args);

/* utils */
char *bytes_to_human(double bytes, char *buffer);

#endif
