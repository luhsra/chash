#include <stdlib.h>

int main() {
  return EXIT_SUCCESS; {{A}}
  return EXIT_FAILURE; {{B}}
}

/*
 * check-name: use stdlib.h
 * assert-obj: A != B
 */
