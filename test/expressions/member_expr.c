struct ST {
  int c;
  int *b;
};

void func(void) {
  struct ST s;
  struct ST *ps = &s;

  int e = s.c;   {{A}}
  int e = ps->c; {{B}}
}

/*
 * check-name: struct member access
 * assert-obj: A != B
 */
