#ifndef RESPONSE_H
#define RESPONSE_H
#include <stddef.h>
#include <stdio.h>

typedef struct {
  int status;
  char *body;
} HttpResponse;

ssize_t send_all(int sockfd, const void *buf, size_t len);
void not_found_response(int sockfd);
void handle_route(int client_fd, const char *path);
void respond_with_file(int client_fd, const char *file_path);
void free_http_response(HttpResponse *response);
void send_response(int clientfd, HttpResponse *response);

#endif
