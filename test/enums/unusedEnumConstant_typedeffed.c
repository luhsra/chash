typedef enum {aa, bb, cc} e_type;     {{A}}
typedef enum {aa, bb, cc, dd} e_type; {{B}}

e_type e = cc;

/*
 * check-name: Adding unused enum constant to typedeffed enum
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */
