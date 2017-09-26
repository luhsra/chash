typedef struct Named {
    int i;
} Named;

typedef struct
Unnamed {{A}}
{
  int i;
} Unnamed_123;

struct Unnamed u; {{A}}
Unnamed_123 u;    {{B}}

/*
 * check-name: typedef struct
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */


