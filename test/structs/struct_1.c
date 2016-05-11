struct foobar {
  int first;
  char second;
  unsigned long third; {{B}}
};

struct foobar foo;

{{A}}

/*
 * check-name: structs with different members
 * assert-obj: A != B
 */
