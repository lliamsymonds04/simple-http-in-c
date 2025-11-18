#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define PORT 8080
#define BACKLOG 5
#define BUFFER_SIZE 4096
#define MAX_HEADERS 32

// global
static int server_fd = -1;

// structs
typedef struct {
  int client_fd;
  struct sockaddr_in client_addr;
} client_info;

typedef struct {
  char *name;
  char *value;
} http_header;

void handle_signal(int sig) {
  printf("\nGracefully shutting down server...\n");
  if (server_fd != -1) {
    close(server_fd);
  }
  exit(0);
}

void free_headers(http_header *headers, int count) {
  for (int i = 0; i < count; i++) {
    free(headers[i].name);
    free(headers[i].value);
  }
  free(headers);
}

void handle_client(int client_fd, struct sockaddr_in *client_addr) {
  char buffer[BUFFER_SIZE];
  char client_ip[INET_ADDRSTRLEN];

  // get client ip address
  inet_ntop(AF_INET, &client_addr->sin_addr, client_ip, sizeof(client_ip));
  printf("Client connected: %s:%d\n", client_ip, ntohs(client_addr->sin_port));

  while (1) {
    ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

    if (bytes_received < 0) {
      perror("recv failed");
      close(client_fd);
      return;
    }

    if (bytes_received == 0) {
      printf("Client disconnected: %s:%d\n", client_ip,
             ntohs(client_addr->sin_port));
      close(client_fd);
      return;
    }

    // Terminate buffer
    buffer[bytes_received] = '\0';
    printf("Received from %s:%d: %s\n", client_ip, ntohs(client_addr->sin_port),
           buffer);

    // Parse request line
    char method[16], path[256], version[16];
    sscanf(buffer, "%15s %255s %15s", method, path, version);

    // Parse headers
    char *line = strtok(buffer, "\r\n");
    int header_count = 0;
    int capacity = 0;
    http_header *headers;
    while ((line = strtok(NULL, "\r\n")) != NULL) {
      if (strlen(line) == 0) {
        break; // End of headers
      }

      char header_name[256], header_value[256];
      sscanf(line, "%255[^:]: %255[^\r\n]", header_name, header_value);

      if (header_count >= capacity) {
        capacity = (capacity == 0) ? 4 : capacity * 2;
        headers = realloc(headers, capacity * sizeof(http_header));
        if (headers == NULL) {
          perror("realloc failed");
          close(client_fd);
          return;
        }
      }

      size_t name_len = strlen(header_name) + 1;
      headers[header_count].name = malloc(name_len);
      strcpy(headers[header_count].name, header_name);

      size_t value_len = strlen(header_value) + 1;
      headers[header_count].value = malloc(value_len);
      strcpy(headers[header_count].value, header_value);

      header_count++;
    }

    char *response = "Message received\n";
    ssize_t bytes_sent = send(client_fd, response, strlen(response), 0);

    free_headers(headers, header_count);

    if (bytes_sent < 0) {
      perror("send failed");
    }
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

int main(int argc, char *argv[]) {
  // signal handling
  signal(SIGINT, handle_signal);
  signal(SIGTERM, handle_signal);

  server_fd = create_server_socket(PORT);

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

    pthread_t thread_id;
    struct client_info *cinfo = malloc(sizeof(struct client_info));
    cinfo->client_fd = client_fd;
    cinfo->client_addr = client_addr;
    pthread_create(&thread_id, NULL, client_thread, cinfo);
    pthread_detach(thread_id);
  }

  close(server_fd);

  return 0;
}
