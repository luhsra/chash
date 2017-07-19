typedef union
Unnamed {{A}}
{
  int i;
} Unnamed_123;

union Unnamed u; {{A}}
Unnamed_123 u;   {{B}}

/*
 * check-name: typedef struct
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */


