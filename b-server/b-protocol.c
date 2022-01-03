#include "b-protocol.h"
#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h> /* htons / htol */

hash_t calc_hash(message_t *message)
{
  if (!message || message->size == 0)
    return 0;

  hash_value hash = 0, c;
  size_t size = message->size;

  for (size_t i = 0; i < size; i++)
  {
    c = (unsigned char)message->bytes[i];
    hash = (hash << 3) + (hash >> (HASH_SIZE * CHAR_BIT - 3)) + c;
  }

  return hash % size;
}

void check_message_hash(message_t *message)
{
  hash_t calculated = calc_hash(message);
  if (calculated != message->hash)
  {
    // error
    message->hash = 0;
  }
}

message_t *create_message(char *bytes, size_t size)
{
  message_t *message = malloc(sizeof(message_t));
  message->bytes = bytes;
  message->size = size;
  message->hash = calc_hash(message);
  return message;
}

void delete_message(message_t *message)
{
  free(message->bytes);
  free(message);
}

request_t *create_request(int fd, int serial, size_t size, int type)
{
  request_t *request = malloc(sizeof(request_t));
  request->fd = fd;
  request->serial = serial;
  request->size = htonl(size);
  request->type = htons(type);
  return request;
}

void decode_request_message(request_message_t *rm)
{
  rm->serial = ntohs(rm->serial);
  rm->size = ntohl(rm->size);
  rm->type = ntohs(rm->type);
}

void delete_request(request_t *request)
{
  free(request);
}

request_message_t *create_request_message(request_t *request)
{
  request_message_t *rm = malloc(sizeof(request_message_t));
  bzero(rm, sizeof(request_message_t));
  rm->serial = request->serial;
  rm->size = htonl(request->size);
  rm->type = htons(request->type);
  return rm;
}

void delete_request_message(request_message_t *rm)
{
  free(rm);
}

thread_args_t *create_thread_args(int sockfd, int thread)
{
  thread_args_t *args = malloc(sizeof(thread_args_t));
  args->fd = sockfd;
  args->thread = thread;
  return args;
}

void delete_thread_args(thread_args_t *args)
{
  free(args);
}
