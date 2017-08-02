static int foo() { return 23; }

static void bar() { foo(); }

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
 * check-name: some static function calls
 * references: bar:export_def_use/functions/static_function_calls.c -> foo:export_def_use/functions/static_function_calls.c, baz -> bar:export_def_use/functions/static_function_calls.c foo:export_def_use/functions/static_function_calls.c, calc -> foo:export_def_use/functions/static_function_calls.c bar:export_def_use/functions/static_function_calls.c baz
 */
