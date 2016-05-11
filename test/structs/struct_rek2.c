struct barfoo;

struct foobar {
  int a;
  struct barfoo *b;
  long c; {{B}}
};

struct barfoo {
  long a;
  struct foobar *b;
  int c;
};

struct barfoo foo;

{{A}}

/*
 * check-name: struct recursive 2; struct with different members but not instantiated
 * assert-ast: A == B
 */
