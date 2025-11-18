#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BACKLOG 5

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

int main(int argc, char *argv[]) {
  int server_fd = create_server_socket(PORT);

  if (server_fd < 0) {
    perror("Failed to create server socket");
    return 1;
  }

  printf("Server listening on port %d\n", PORT);

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

    printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    close(client_fd);
  }

  close(server_fd);

  return 0;
}
