#include <stdint.h> {{A}}

int variable;
{{B}}

/*
 * check-name: unused include
 * assert-obj: A == B
 */
