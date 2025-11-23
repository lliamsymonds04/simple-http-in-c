#include "http.h"

#define PORT 8080
#define BUFFER_SIZE 4096

int main() {
  // signal handling
  http_init();
  http_listen(PORT);

  return 0;
}
