struct foobar {
  int first;
  char second;
  unsigned long third; {{B}}
};

{{A}}
struct foobar foo;

/*
 * check-name: structs with different members
 * assert-obj: A != B
 */
