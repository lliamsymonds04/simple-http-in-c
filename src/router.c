#include "router.h"
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_ROUTES 100

// storage for routes
static Route routes[MAX_ROUTES];
static int route_count = 0;

void register_route(const char *pattern, RouteHandler handler) {
  if (route_count >= MAX_ROUTES) {
    fprintf(stderr, "Max route limit reached\n");
    return;
  }

  routes[route_count].pattern = strdup(pattern);
  routes[route_count].handler = handler;
  route_count++;
}

int match_and_handle_route(const char *path, HttpResponse **handler_result) {
  char path_copy[512];
  char *query_string = NULL;
  strcpy(path_copy, path);

  char *question_mark = strchr(path_copy, '?');
  if (question_mark) {
    *question_mark = '\0';
    query_string = question_mark + 1;
  }

  for (int i = 0; i < route_count; i++) {
    if (strcmp(routes[i].pattern, path_copy) == 0) {
      UrlParams *params = malloc(sizeof(UrlParams));
      parse_url_params(query_string, params);
      *handler_result = routes[i].handler(params);
      free_url_params(params);

      return 1; // Route matched and handled
    }
  }

  return 0;
}
