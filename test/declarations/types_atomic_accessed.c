_Atomic double a; {{A}}
double a;         {{B}}

void func(void) {
  a = 0;
}

/*
 * check-name: atomic types (accessed)
 * assert-obj: A != B
 */
