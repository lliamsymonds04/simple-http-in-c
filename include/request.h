#ifndef REQUEST_H
#define REQUEST_H
#include <stddef.h>

typedef struct {
  char *name;
  char *value;
} http_header;

typedef struct {
  char method[8];
  char path[256];
  char version[16];
} HttpRequest;

const char *get_mime_type(const char *path);
int parse_headers(char *buffer, http_header **headers, int *count);
void free_headers(http_header *headers, int count);
int parse_request(int client_fd, char *buffer, size_t buffer_size,
                  HttpRequest *req);

#endif
