struct St {
  int a;
  char b;
  char *c;
};

struct St2 {
  int q;
  struct St st;
};

struct St2 st = {42, {5, 'a', "Hello"}}; {{A}}
struct St2 st = {42, {5, 'b', "World"}}; {{B}}

/*
 * check-name: Initlist struct 2 (nested)
 * assert-obj: A != B
 */
