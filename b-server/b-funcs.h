#ifndef B_FUNCS_H
#define B_FUNCS_H
#include <stdio.h>

#define DEFAULT_PORT    8080
#define DEFAULT_SERVER  "127.0.0.1"

typedef unsigned int hash_value;

#define HASH_SIZE sizeof(hash_value)

typedef union 
{
	hash_value value;
	char encoded[HASH_SIZE];
} hash_t;

typedef struct 
{
	char *bytes;
	size_t size;
  hash_t hash;
} message_t;

hash_value calc_hash(message_t *message, int hash_included);
hash_value get_hash(message_t *message);

message_t *create_message(char *bytes, size_t size);

#endif
