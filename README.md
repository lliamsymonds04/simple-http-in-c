# Simple HTTP Server

This is a pretty simple HTTP server written in C to deepen my understanding of the HTTP protocol and socket programming. It supports basic HTTP GET requests and serves static files from a specified directory. Additionally, it allows for basic routing.

## Usage

The most basic usage of this HTTP server is as follows:

```c
#include "http_server.h"

http_init();
http_listen("<port>");
```

This will start an HTTP server listening on the specified port. This will serve files in the `public` directory

example:

```
curl http://localhost:<port>/kitty.jpg
```

This shows a picture of a cat.

### Routing

```

// Handler function
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

// Register route hello to the handler
http_register_route("/hello", hello_handler);
```

```
curl http://localhost:<port>/hello?name=Lliam
// Output: Hello Lliam!
```

### Compile the code

```bash
make
```

### Start the server

```bash
./http_server <port>
```

## Recommended Readings

Some resources that helped me build this project:

[Beej's TCP](https://beej.us/guide/bgnet/html/split/index.html)

[HTTP/1.1](https://www.rfc-editor.org/rfc/rfc9110)

[Mozilla Get Request](https://developer.mozilla.org/en-US/docs/Web/HTTP/Reference/Methods/GET)
