#include "include_1.h" {{A}}
{{B}}

/*
 * check-name: unused Decls in include
 * This include seems to change the object-file, but the Hash should not be changed. (compare to in_c_1.c)
 * B == A
 */
