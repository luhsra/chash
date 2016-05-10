#include <stddef.h>

struct test {
  int a1;
  int a2;
};

void func() {
  unsigned int a = offsetof(struct test, a2); {{A}}
  unsigned int a = 4;                         {{B}}
}

/*
 * check-name: offsetof struct
 * obj-not-diff: might
 * assert-ast: A != B
 * assert-obj: A == B
 */
