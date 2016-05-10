struct foobar {
  int gelb;
};

void func(void) {
  struct foobar* a = (struct cc_trie*) (sizeof(*a)); {{A}}
  struct foobar* a = (struct foobar*) (sizeof(*a));  {{B}}
}

/*
 * check-name: size of the own declaration --> Seg-Fault?
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */
//TODO: Testcase draus machen!
