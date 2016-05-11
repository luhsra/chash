struct foobar {
  int l;
};

struct foobar *barfoo() {
  struct foobar foo = {.l = 1};
  struct foobar *pfoo = &foo;
  return &foo; {{A}}
  return pfoo; {{B}}
}

/*
 * check-name: return recordptr
 * assert-obj: A != B
 */
