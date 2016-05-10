struct foobar {
  int *first;
  char *second;
  unsigned long third; {{B}}
};

struct foobar *foo; {{A}}

int val;

/*
 * check-name: structs with different members and ptr to struct (no instance)
 * assert-obj: A != B
 */

