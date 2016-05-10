struct St {
  int a;
  char b;
  char *c;
};

struct St st = {5, 'a', "Hello"}; {{A}}
struct St st = {5, 'b', "World"}; {{B}}

/*
 * check-name: Initlist struct 1
 * assert-obj: A != B
 */
