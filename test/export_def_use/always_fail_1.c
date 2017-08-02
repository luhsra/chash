
int foo() {
  return 1;
}

/*
 * check-name: always fail 1
 * references: foo -> bar
 * check-known-to-fail: true
 */
