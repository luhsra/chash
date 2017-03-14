int foo() { return 23; }

void bar() { foo(); }

int baz() {
  bar();
  return foo();
}

int calc() {
  bar();
  int i = foo() / baz() + 42;
  return i;
}

/*
 * check-name: some function calls
 * references: bar -> foo, baz -> bar foo, calc -> foo bar baz
 */
