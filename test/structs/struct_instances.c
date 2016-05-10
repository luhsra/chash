struct One {
  int a;
  char b;
  char *c;
};

struct Two {
  int q;
  struct One one;
};

struct Two a; {{A}}
struct Two b; {{B}}

/*
 * check-name: struct instances with different names
 * assert-obj: A != B
 */
