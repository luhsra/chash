
struct {{A}}
struct {{B}}
union {{C}}
union {{D}}
foo {
  int i;
  struct bar {
    int j; {{A}}
    int j; {{D}}
  } x;
  int j; {{B}}
  int j; {{C}}
};

struct foo f; {{A}}
struct foo f;{{B}}
union foo f;{{C}}
union foo f;{{D}}

/*
 * check-name: nested structs
 * obj-not-diff: true
 * assert-ast: A != B, A != C, B != C, D != A, D != C, D != B
 */
