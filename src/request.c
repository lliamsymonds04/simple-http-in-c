#include "request.h"
#include <arpa/inet.h>
#include <ctype.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

int parse_request(int client_fd, char *buffer, size_t buffer_size,
                  HttpRequest *req) {
  ssize_t bytes_received = recv(client_fd, buffer, buffer_size - 1, 0);

  if (bytes_received < 0) {
    return -1;
  } else if (bytes_received == 0) {
    return 0; // Connection closed
  }

  buffer[bytes_received] = '\0';

  // Parse request line
  char *line_end = strstr(buffer, "\r\n");
  if (!line_end) {
    return -1;
  }

  *line_end = '\0';
  sscanf(buffer, "%15s %255s %15s", req->method, req->path, req->version);
  *line_end = '\r'; // Restore for header parsing

  return 1;
}

char *extract_query_string(const char *path) {
  const char *query_start = strchr(path, '?');
  if (!query_start) {
    return NULL;
  }
  return strdup(query_start + 1);
}

void parse_url_params(const char *query_string, UrlParams *params) {
  params->count = 0;
  params->keys = NULL;
  params->values = NULL;

  if (!query_string || query_string[0] == '\0') {
    return;
  }

  char *query_dup = strdup(query_string);

  char *temp = strdup(query_dup);
  char *pair = strtok(temp, "&");
  while (pair != NULL) {
    params->count++;
    pair = strtok(NULL, "&");
  }
  free(temp);

  params->keys = malloc(params->count * sizeof(char *));
  params->values = malloc(params->count * sizeof(char *));

  // Second pass to fill keys and values
  int i = 0;
  pair = strtok(query_dup, "&");
  while (pair != NULL) {
    char *equal_sign = strchr(pair, '=');
    if (equal_sign) {
      *equal_sign = '\0';
      params->keys[i] = strdup(pair);
      params->values[i] = strdup(equal_sign + 1);
      printf("Parsed param: %s = %s\n", params->keys[i], params->values[i]);
    } else {
      params->keys[i] = strdup(pair);
      params->values[i] = strdup("");
    }
    i++;
    pair = strtok(NULL, "&");
  }

  free(query_dup);
}

void url_decode(char *dst, const char *src) {
  while (*src) {
    if (*src == '%' && isxdigit(src[1]) && isxdigit(src[2])) {
      char hex[3] = {src[1], src[2], '\0'};
      *dst++ = (char)strtol(hex, NULL, 16);
      src += 3;
    } else if (*src == '+') {
      *dst++ = ' ';
      src++;
    } else {
      *dst++ = *src++;
    }
  }
  *dst = '\0';
}

void free_url_params(UrlParams *params) {
  free(params->keys);
  free(params->values);
  free(params);
};
