/* The gcc uses an internal counter for names he has seen. This
 * counter is used to name static local variables */

typedef int UNUSED; {{A}}
{{B}}

void foo() {
    static int A;
    (void) A;
}

/*
 * check-name: internal linkage name changes
 * compile-command: /usr/bin/gcc
 * compile-flags: -fdata-sections -Wall
 * B == A
 */
