#include "http.h"
#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char *argv[]) {
  http_init();
  // connect routes
  http_route("/add", handle_addition);

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
