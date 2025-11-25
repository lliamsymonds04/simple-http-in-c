#ifndef ROUTER_H
#define ROUTER_H

#include "request.h"
#include "response.h"

typedef HttpResponse *(*RouteHandler)(UrlParams *params);

typedef struct {
  char *pattern;
  RouteHandler handler;
} Route;

void register_route(const char *pattern, RouteHandler handler);
int match_and_handle_route(const char *path, HttpResponse **result);

#endif
