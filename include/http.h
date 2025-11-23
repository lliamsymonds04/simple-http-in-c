#ifndef HTTP_H
#define HTTP_H

// Initializes the global HTTP server state
void http_init();

// Starts the server loop on the specified port
int http_listen(int port);

// cleans up and shuts down the HTTP server
void http_shutdown();

#endif
