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

const char *reason_phrase(int code) {
  switch (code) {
  case 200:
    return "OK";
  case 400:
    return "Bad Request";
  case 404:
    return "Not Found";
  case 500:
    return "Internal Server Error";
  default:
    return ""; // generic fallback
  }
}

void send_response(int clientfd, HttpResponse *response) {
  const char *reason = reason_phrase(response->status);
  const int body_length = strlen(response->body);

  char header[256];
  int header_len = sprintf(header,
                           "HTTP/1.1 %d %s\r\n"
                           "Content-Length: %d\r\n"
                           "Content-Type: text/plain\r\n"
                           "\r\n",
                           response->status, reason, body_length);

  if (header_len < 0) {
    return;
  }

  size_t total_length = header_len + body_length;

  char *buffer = malloc(total_length);
  if (!buffer) {
    return;
  }

  memcpy(buffer, header, header_len);
  memcpy(buffer + header_len, response->body, body_length);

  send_all(clientfd, buffer, total_length);

  free(buffer);
}

void not_found_response(int sockfd) {
  HttpResponse *response = malloc(sizeof(HttpResponse));
  response->status = 404;
  response->body = strdup("<html><body><h1>404 Not Found</h1></body></html>");
  send_response(sockfd, response);
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

void free_http_response(HttpResponse *response) {
  if (response) {
    if (response->body) {
      free(response->body);
    }
    free(response);
  }
}
