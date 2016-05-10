#include <stdio.h>

void f() {
  {{A}}
  {{B}}
}

/*
 * check-name: included not used library
 * assert-obj: A == B
 */
