typedef enum{aa, bb, cc} e_type; {{A}}
typedef enum{aa, bb, cc, dd} e_type; {{B}}

e_type e = cc;
/*
 * check-name: Adding unused enum constatnt
 * assert-ast: A != B
 * obj-not-diff: yes
 */
