struct foobar {
  int var;
};

void func(void) {
  struct foobar a = {sizeof(a)};                    {{A}}
  struct foobar a = {sizeof(a.var)};                {{B}}
  struct foobar a = {4};                            {{C}}
}

/*
 * check-name: size of the own declaration
 * obj-not-diff: yes
 * assert-ast: A != B, A != C, B != C
 * assert-obj: A == B, A == C, B == C
 */
