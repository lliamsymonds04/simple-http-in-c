#ifndef RESPONSE_H
#define RESPONSE_H
#include <stddef.h>
#include <stdio.h>

ssize_t send_all(int sockfd, const void *buf, size_t len);
void not_found_response(int sockfd);
void handle_route(int client_fd, const char *path);

#endif
