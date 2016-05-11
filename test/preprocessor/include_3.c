#include "headers/include_3A.h" {{A}}
#include "headers/include_3B.h" {{B}}
int hi = 7;                     {{C}}

int a() {
  int a = hi;
  return 0;
}

/*
 * check-name: include stuff
 * assert-ast: B == C
 * assert-obj: A != B, A != C
 */
