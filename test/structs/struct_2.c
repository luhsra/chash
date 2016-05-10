struct foobar {
  int first;
  char second;
  unsigned long third; {{B}}
};
{{A}}
int var;

/*
 * check-name: unused structs with different members
 * assert-ast: A == B
 */
