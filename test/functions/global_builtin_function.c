int callee(int);

void caller(void) {
  int a = 8;
  callee(a);
}

int callee(int a) {
  return a + 1; {{A}}
  return a - 1; {{B}}
}

int main() {
  caller();
}

/*
 * check-name: Function-Test-1
 * assert-obj: A != B
 */
