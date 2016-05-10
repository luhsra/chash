int f(void) {
  long a = 1;      {{A}}
  long long a = 1; {{B}}
  return a;
}

/*
 * check-name: long vs long long
 * obj-not-diff: reasonable, long same as long long
 * assert-ast: A != B
 * assert-obj: A == B
 */

