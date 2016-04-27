typedef enum{aa, bb, cc} e_type; {{A}}
typedef enum{aa, bb, cc, dd} e_type; {{B}}

e_type e = cc;
/*
 * check-name: Adding unused enum constatnt
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */
