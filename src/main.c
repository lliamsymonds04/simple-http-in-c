#include <arpa/inet.h>
#include <fcntl.h>
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

// global
static int server_fd = -1;

// structs
struct client_info {
  int client_fd;
  struct sockaddr_in client_addr;
};

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

int parse_headers(char *buffer, http_header **headers, int *count) {
  char *line = strtok(buffer, "\r\n");
  int capacity = 0;
  while ((line = strtok(NULL, "\r\n")) != NULL) {
    if (strlen(line) == 0) {
      break; // End of headers
    }

    char header_name[256], header_value[256];
    sscanf(line, "%255[^:]: %255[^\r\n]", header_name, header_value);

    if (*count >= capacity) {
      capacity = (capacity == 0) ? 4 : capacity * 2;
      http_header *temp = realloc(*headers, capacity * sizeof(http_header));
      if (temp == NULL) {
        perror("realloc failed");
        return -1;
      }
      *headers = temp;
    }

    size_t name_len = strlen(header_name) + 1;
    (*headers)[*count].name = malloc(name_len);
    if ((*headers)[*count].name == NULL) {
      perror("malloc failed");
      return -1;
    }
    strcpy((*headers)[*count].name, header_name);

    size_t value_len = strlen(header_value) + 1;
    (*headers)[*count].value = malloc(value_len);
    if ((*headers)[*count].value == NULL) {
      perror("malloc failed");
      return -1;
    }
    strcpy((*headers)[*count].value, header_value);

    *count = *count + 1;
  }

  return 0;
}

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

void handle_client(int client_fd, struct sockaddr_in *client_addr) {
  char buffer[BUFFER_SIZE];
  char client_ip[INET_ADDRSTRLEN];

  // get client ip address
  inet_ntop(AF_INET, &client_addr->sin_addr, client_ip, sizeof(client_ip));

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
    http_header *headers = NULL;
    int header_count = 0;

    if (parse_headers(buffer, &headers, &header_count) < 0) {
      perror("Failed to parse headers");
      close(client_fd);
      return;
    }

    // Build response
    if (strcmp(method, "GET") == 0) {
      // join public directory with path
      char full_path[512];
      if (strcmp(path, "/") == 0) {
        sprintf(full_path, "public/index.html");
      } else {
        sprintf(full_path, "public%s", path);
      }

      // attempt to open the requested file
      int file_fd = open(full_path, O_RDONLY);
      if (file_fd < 0) {
        // File not found, respond with 404
        not_found_response(client_fd);
      } else {
        // get the file size
        off_t file_size = lseek(file_fd, 0, SEEK_END);
        lseek(file_fd, 0, SEEK_SET);

        // build header
        const char *mime_type = get_mime_type(full_path);
        char *header = (char *)malloc(BUFFER_SIZE * sizeof(char));
        int header_len = sprintf(header,
                                 "HTTP/1.1 200 OK\r\n"
                                 "Content-Type: %s\r\n"
                                 "Content-Length: %zu\r\n"
                                 "\r\n",
                                 mime_type, file_size);

        send_all(client_fd, header, header_len);
        free_headers(headers, header_count);

        char buff[BUFFER_SIZE];
        ssize_t bytes_read;
        while (bytes_read = read(file_fd, buff, sizeof(buff)), bytes_read > 0) {
          if (send_all(client_fd, buff, bytes_read) < 0) {
            break;
          }
        }

        close(file_fd);
      }
    } else {
      not_found_response(client_fd);
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
