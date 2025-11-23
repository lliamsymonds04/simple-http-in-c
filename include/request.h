#ifndef REQUEST_H
#define REQUEST_H

typedef struct {
  char *name;
  char *value;
} http_header;

const char *get_mime_type(const char *path);
int parse_headers(char *buffer, http_header **headers, int *count);
void free_headers(http_header *headers, int count);

#endif
