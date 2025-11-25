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
  printf("Addition result: %s + %s = %s\n", a_raw ? a_raw : "0",
         b_raw ? b_raw : "0", response->body);
  return response;
}

int main() {
  // signal handling
  http_init();
  http_route("/add", handle_addition);
  http_listen(DEFAULT_PORT);

  return 0;
}
