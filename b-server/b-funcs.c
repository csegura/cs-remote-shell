#include "b-funcs.h"
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

hash_value calc_hash(message_t *message, int hash_included)
{

  if (message->size == 0)
    return 0;

  hash_value hash = 0, c;
  size_t size = message->size - (hash_included ? HASH_SIZE : 0);

  for (int i = 0; i < size; i++)
  {
    c = (unsigned char)message->bytes[i];
    hash = (hash << 3) + (hash >> (HASH_SIZE * CHAR_BIT - 3)) + c;
  }
  printf("%u %d\n", hash, size);
  return hash % size;
}

hash_value get_hash(message_t *message)
{
  hash_t hash;

  memcpy(hash.encoded,
         &message->bytes[message->size - HASH_SIZE],
         HASH_SIZE);

  return hash.value;
}

message_t *create_message(char *bytes, size_t size)
{
  message_t *message = malloc(sizeof(message_t));
  message->bytes = bytes;
  message->size = size;
  message->hash.value = calc_hash(message, 0);
  return message;
}