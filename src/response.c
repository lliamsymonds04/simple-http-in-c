#include "response.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUFFER_SIZE 4096

const char *get_mime_type(const char *path) {
  const char *ext = strrchr(path, '.');
  if (!ext) {
    return "application/octet-stream";
  }
  if (strcmp(ext, ".html") == 0 || strcmp(ext, ".htm") == 0) {
    return "text/html";
  } else if (strcmp(ext, ".css") == 0) {
    return "text/css";
  } else if (strcmp(ext, ".js") == 0) {
    return "application/javascript";
  } else if (strcmp(ext, ".png") == 0) {
    return "image/png";
  } else if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
    return "image/jpeg";
  } else if (strcmp(ext, ".gif") == 0) {
    return "image/gif";
  } else if (strcmp(ext, ".txt") == 0) {
    return "text/plain";
  } else {
    return "application/octet-stream";
  }
}

ssize_t send_all(int sockfd, const void *buf, size_t len) {
  size_t total_sent = 0;
  const char *buffer = (const char *)buf;

  while (total_sent < len) {
    ssize_t bytes_sent = send(sockfd, buffer + total_sent, len - total_sent, 0);
    if (bytes_sent < 0) {
      return -1;
    }
    total_sent += bytes_sent;
  }

  return total_sent;
}

void not_found_response(int sockfd) {
  const char *body = "<html><body><h1>404 Not Found</h1></body></html>";
  const char *header = "HTTP/1.1 404 Not Found\r\n"
                       "Content-Type: text/html\r\n"
                       "Content-Length: %zu\r\n"
                       "\r\n";

  char response[BUFFER_SIZE];

  size_t body_length = strlen(body);
  size_t response_length = snprintf(response, BUFFER_SIZE, header, body_length);
  strcat(response, body);
  response_length += body_length;

  send_all(sockfd, response, response_length);
}

void respond_with_file(int client_fd, const char *file_path) {
  int file_fd = open(file_path, O_RDONLY);
  if (file_fd < 0) {
    // File not found, respond with 404
    not_found_response(client_fd);
  } else {
    // get the file size
    off_t file_size = lseek(file_fd, 0, SEEK_END);
    lseek(file_fd, 0, SEEK_SET);

    // build header
    const char *mime_type = get_mime_type(file_path);
    char *header = (char *)malloc(BUFFER_SIZE * sizeof(char));
    int header_len = sprintf(header,
                             "HTTP/1.1 200 OK\r\n"
                             "Content-Type: %s\r\n"
                             "Content-Length: %zu\r\n"
                             "\r\n",
                             mime_type, file_size);

    send_all(client_fd, header, header_len);

    char buff[BUFFER_SIZE];
    ssize_t bytes_read;
    while (bytes_read = read(file_fd, buff, sizeof(buff)), bytes_read > 0) {
      if (send_all(client_fd, buff, bytes_read) < 0) {
        break;
      }
    }

    close(file_fd);
  }
}

void handle_route(int client_fd, const char *route) {
  if (strcmp(route, "/") == 0) {
    // respond with index page
    respond_with_file(client_fd, "public/index.html");
  } else {
    char file_path[512];
    sprintf(file_path, "public%s", route);
    respond_with_file(client_fd, file_path);
  }
}
