#include "header_extern_2.h" {{A}}
{{B}}

int foo() {
  return 0;
}

/*
 * check-name: implementation of included extern function declaration ()
 * assert-obj: A == B
 */
