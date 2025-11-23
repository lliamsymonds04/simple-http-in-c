#include "request.h"
#include <arpa/inet.h>
#include <pthread.h>
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

void free_headers(http_header *headers, int count) {
  for (int i = 0; i < count; i++) {
    free(headers[i].name);
    free(headers[i].value);
  }
  free(headers);
}

int parse_headers(char *buffer, http_header **headers, int *count) {
  int capacity = 0;
  char *current = buffer;

  // Skip the request line (first line)
  char *line_end = strstr(current, "\r\n");
  if (!line_end) {
    return -1;
  }
  current = line_end + 2; // Move past \r\n

  // Parse each header line
  while (*current != '\0') {
    // Find end of current line
    line_end = strstr(current, "\r\n");
    if (!line_end) {
      break;
    }

    // Calculate line length
    size_t line_len = line_end - current;

    // Empty line signals end of headers
    if (line_len == 0) {
      break;
    }

    // Find the colon separator
    char *colon = memchr(current, ':', line_len);
    if (!colon) {
      current = line_end + 2;
      continue;
    }

    // Extract header name
    size_t name_len = colon - current;
    char *header_name = malloc(name_len + 1);
    if (!header_name) {
      perror("malloc failed");
      return -1;
    }
    memcpy(header_name, current, name_len);
    header_name[name_len] = '\0';

    // Skip colon and spaces
    char *value_start = colon + 1;
    while (value_start < line_end &&
           (*value_start == ' ' || *value_start == '\t')) {
      value_start++;
    }

    // Extract header value
    size_t value_len = line_end - value_start;
    char *header_value = malloc(value_len + 1);
    if (!header_value) {
      free(header_name);
      perror("malloc failed");
      return -1;
    }
    memcpy(header_value, value_start, value_len);
    header_value[value_len] = '\0';

    // Expand array if needed
    if (*count >= capacity) {
      capacity = (capacity == 0) ? 4 : capacity * 2;
      http_header *temp = realloc(*headers, capacity * sizeof(http_header));
      if (temp == NULL) {
        free(header_name);
        free(header_value);
        perror("realloc failed");
        return -1;
      }
      *headers = temp;
    }

    (*headers)[*count].name = header_name;
    (*headers)[*count].value = header_value;
    *count = *count + 1;

    // Move to next line
    current = line_end + 2;
  }

  return 0;
}

int parse_request(int client_fd, HttpRequest *req) {
  char buffer[BUFFER_SIZE];
  ssize_t bytes_received = recv(client_fd, buffer, sizeof(buffer) - 1, 0);

  if (bytes_received <= 0) {
    return -1;
  }

  // Terminate buffer
  buffer[bytes_received] = '\0';

  // Parse request line
  char *line_end = strstr(buffer, "\r\n");
  if (!line_end) {
    return -1;
  }

  *line_end = '\0';
  sscanf(buffer, "%15s %255s %15s", req->method, req->path, req->version);
  *line_end = '\r'; // Restore for header parsing

  return 0;
}
