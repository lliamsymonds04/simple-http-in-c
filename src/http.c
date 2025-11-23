#include "http.h"
#include "server.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Global server file descriptor
int server_fd = -1;

void http_shutdown() {
  if (server_fd != -1) {
    close(server_fd);
    server_fd = -1;
  }
};

void handle_signal() {
  printf("\nGracefully shutting down server...\n");
  http_shutdown();
  exit(0);
}

void http_init() {
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);
}

int http_listen(int port) {
  server_fd = create_server_socket(port);
  if (server_fd < 0) {
    perror("Failed to create server socket");
    return 1;
  }

  printf("Server listening on port %d\n", port);
  server_listen(server_fd);

  return 0;
}
