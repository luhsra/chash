int ext = 0;

int other_foo() { return 0xdeadbeef; }

/*
 * check-name: no references
 * no-references: ext other_foo
 */
