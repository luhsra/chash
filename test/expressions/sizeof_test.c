void func(void) {
  int var;
  struct st {
    int a;
    int b;
  };
  unsigned int s = sizeof(struct st); {{A}}
  unsigned int s = sizeof(int);       {{B}}
  unsigned int s = sizeof(var);       {{C}}
}

/*
 * check-name: sizeof
 * obj-not-diff: B == C
 * assert-ast: B != C
 * assert-obj: B == C, A != B, A != C
 */
