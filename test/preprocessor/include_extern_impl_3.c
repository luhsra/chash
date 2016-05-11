#include "header_extern_1.h" {{A}}
{{B}}

int foo() {
  return 0;
}


/*
 * check-name: Include with different extern function declaration
 * obj-not-diff: void parameter != no parameter
 * assert-ast: A != B
 * assert-obj: A == B
 */
