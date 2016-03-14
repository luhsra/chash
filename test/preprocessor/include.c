#include "header.h" {{A}}
{{B}}

int foo() {
    return 0;
}


/*
 * check-name: Include with function extern
 * obj-not-diff: yes
 * B != A
 */
