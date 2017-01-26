typedef float vector_t  __attribute__((__vector_size__(16))); {{A}}
typedef float vector_t  __attribute__((ext_vector_type(16))); {{B}}
typedef float vector_t; {{C}}

vector_t variable;

/*
 * check-name: vector types should be hashed
 * oj-not-diff: yes
 * assert-ast: A != B, B != C, C != A
 */
