#include "router.h"
#include "response.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void register_route(const char *pattern, RouteHandler handler);

void match_and_handle_route(int client_fd, const char *path) {
  if (strcmp(path, "/") == 0) {
    // respond with index page
    respond_with_file(client_fd, "public/index.html");
  } else {
    char file_path[512];
    sprintf(file_path, "public%s", path);
    respond_with_file(client_fd, file_path);
  }
}
