#include "http.h"
#include "request.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEFAULT_PORT 8080
#define BUFFER_SIZE 4096

HttpResponse *handle_addition(UrlParams *params) {
  char *a_raw = (char *)get_param_value(params, "a");
  char *b_raw = (char *)get_param_value(params, "b");

  double a = a_raw ? atof(a_raw) : 0;
  double b = b_raw ? atof(b_raw) : 0;

  HttpResponse *response = malloc(sizeof(HttpResponse));
  response->status = 200;
  response->body = malloc(32);
  if (!response->body) {
    free(response);
    return NULL;
  }
  snprintf(response->body, 32, "%f", a + b);
  return response;
}

HttpResponse *hello_handler(UrlParams *request) {
  char *username = (char *)get_param_value(request, "name");
  if (!username) {
    username = "Guest";
  }

  size_t len = strlen(username) + 8;

  HttpResponse *response = malloc(sizeof(HttpResponse));
  response->body = malloc(len);
  if (!response->body) {
    free(response);
    return NULL;
  }

  response->status = 200;
  snprintf(response->body, len, "Hello %s!", username);

  return response;
}

// Register route
int main(int argc, char *argv[]) {
  http_init();
  // connect routes
  http_register_route("/add", handle_addition);
  http_register_route("/hello", hello_handler);

  // listen
  int port = DEFAULT_PORT;
  if (argc > 1) {
    int user_port = atoi(argv[1]);
    if (user_port > 0 && user_port < 65536) {
      port = user_port;
    }
  }
  http_listen(port);

  return 0;
}
