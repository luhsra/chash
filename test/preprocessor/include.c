#include "header.h" {{A}}
{{B}}

int foo() {
    return 0;
}


/*
 * check-name: Include with function extern
 * obj-not-diff: void parameter != no parameter
 * assert-ast: A != B
 * assert-obj: A == B
 */
