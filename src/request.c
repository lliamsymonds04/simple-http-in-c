#include "request.h"
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
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
