struct foobar {
  int first;
  char second;
  unsigned long third; {{B}}
};

int var;

{{A}}

/*
 * check-name: unused structs with different members
 * assert-ast: A == B
 */
