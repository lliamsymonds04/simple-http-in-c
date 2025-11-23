#include "request.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
