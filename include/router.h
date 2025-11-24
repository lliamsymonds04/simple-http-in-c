#ifndef ROUTER_H
#define ROUTER_H

#include "request.h"

typedef void (*RouteHandler)(int client_fd, UrlParams *params);

typedef struct {
  char *pattern;
  RouteHandler handler;
} Route;

void register_route(const char *pattern, RouteHandler handler);
int match_and_handle_route(int client_fd, const char *path);

#endif
