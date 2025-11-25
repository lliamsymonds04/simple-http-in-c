#ifndef HTTP_H
#define HTTP_H
#include "router.h"

// Initializes the global HTTP server state
void http_init();

// Starts the server loop on the specified port
int http_listen(int port);

// Assign a handler to a route pattern
int http_register_route(const char *pattern, RouteHandler handler);

// Get the param value by key from UrlParams
void *get_param_value(UrlParams *params, const char *key);

// cleans up and shuts down the HTTP server
void http_shutdown();

#endif
