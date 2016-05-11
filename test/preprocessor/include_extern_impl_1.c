#include "header_extern_1.h" {{A}}
{{B}}

int foo(void) {
  return 0;
}

/*
 * check-name: implementation of included extern function declaration (void)
 * assert-obj: A == B
 */
