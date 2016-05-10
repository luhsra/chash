struct foobar {
  int l;
};

struct foobar *barfoo() {
  struct foobar foo = {.l = 1};
  return &foo;
}

{{A}}
{{B}}

/*
 * check-name: return recordptr
 * assert-ast: A == B
 */
//TODO: Testcase draus machen!
