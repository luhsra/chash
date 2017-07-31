
int foo() {
  return 1;
}

/*
 * check-name: always fail 1
 * references: foo:export_def_use/always_fail_1.c -> bar:export_def_use/always_fail_1.c
 * check-known-to-fail: true
 */
