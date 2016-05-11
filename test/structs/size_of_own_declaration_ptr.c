struct foobar {
  int var;
};

void func(void) {
  struct foobar* a = (void*) (sizeof(*a));          {{A}}
  struct foobar* a = (struct foobar*) (sizeof(*a)); {{B}}
  struct foobar* a = (struct foobar*) (4);          {{C}}
}

/*
 * check-name: size of the own declaration ptr --> Seg-Fault?
 * obj-not-diff: yes
 * assert-ast: A != B, A != C, B != C
 * assert-obj: A == B, A == C, B == C
 */
