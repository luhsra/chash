void f() {
  static int a; {{A}}
  int a; {{B}}
// pay attention wether obj-file is even created:
// the next line enforces this!
  a = 3;
}

/*
 * check-name: static variable in function
 * assert-obj: A != B
 */
