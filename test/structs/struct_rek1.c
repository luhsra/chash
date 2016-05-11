struct barfoo;

struct foobar {
  int a;
  struct barfoo *b;
  long c;
};

struct barfoo {
  long a;
  struct foobar *b;
  int c; {{B}}
};

struct foobar foo;

{{A}}

/*
 * check-name: struct recursive 1; struct with different members but not instantiated
 * assert-ast: A == B
 */
