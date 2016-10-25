#include <stdint.h> {{A}}

void f() {
  {{B}}
}

/*
 * check-name: unused include
 * assert-obj: A == B
 */
