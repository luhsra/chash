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
 * references: bar:export_def_use/functions/function_calls.c -> foo:export_def_use/functions/function_calls.c, baz:export_def_use/functions/function_calls.c -> bar:export_def_use/functions/function_calls.c foo:export_def_use/functions/function_calls.c, calc:export_def_use/functions/function_calls.c -> foo:export_def_use/functions/function_calls.c bar:export_def_use/functions/function_calls.c baz:export_def_use/functions/function_calls.c
 */
