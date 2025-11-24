#include "server.h"
#include "request.h"
#include "response.h"
#include "router.h"
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BACKLOG 5
#define BUFFER_SIZE 4096
#define SHOW_REQUEST 1

// structs
struct client_info {
  int client_fd;
  struct sockaddr_in client_addr;
};

void handle_client(int client_fd, struct sockaddr_in *client_addr) {
  char client_ip[INET_ADDRSTRLEN];

  // get client ip address
  inet_ntop(AF_INET, &client_addr->sin_addr, client_ip, sizeof(client_ip));

  while (1) {
    HttpRequest *req = malloc(sizeof(HttpRequest));
    char buffer[BUFFER_SIZE];
    int status = parse_request(client_fd, buffer, BUFFER_SIZE, req);
    if (status <= 0) {
      if (status == -1) {
        perror("Failed to parse request");
      }
      free(req);
      close(client_fd);
      return;
    } else {
      printf("Received from %s:%d\n %s\n", client_ip,
             ntohs(client_addr->sin_port), buffer);
    }

    // Build response
    if (strcmp(req->method, "GET") == 0) {
      if (!match_and_handle_route(client_fd, req->path)) {
        //  not_found_response(client_fd);
        handle_route(client_fd, req->path);
      }
    } else {
      not_found_response(client_fd);
    }

    free(req);
  }
}

void *client_thread(void *arg) {
  struct client_info *cinfo = (struct client_info *)arg;
  handle_client(cinfo->client_fd, &cinfo->client_addr);
  free(cinfo);
  return NULL;
}

int create_server_socket(int port) {
  int sockfd;
  struct sockaddr_in server_addr;
  int opt = 1;

  // create socket
  sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd < 0) {
    return -1;
  }

  // set socket options
  if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
    close(sockfd);
    return -1;
  }

  // configure server address struct
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  // bind socket to the specified port
  if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    close(sockfd);
    return -1;
  }

  // listen for incoming connections
  if (listen(sockfd, BACKLOG) < 0) {
    close(sockfd);
    return -1;
  }

  return sockfd;
}

void server_listen(int server_fd) {
  // accept and handle connections
  while (1) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &client_len);

    if (client_fd < 0) {
      perror("Failed to accept connection");
      continue;
    }

    pthread_t thread_id;
    struct client_info *cinfo = malloc(sizeof(struct client_info));
    cinfo->client_fd = client_fd;
    cinfo->client_addr = client_addr;
    pthread_create(&thread_id, NULL, client_thread, cinfo);
    pthread_detach(thread_id);
  }
}
