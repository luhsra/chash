enum my_enum {aa, bb, cc}; {{A}}
enum my_enum {bb, aa, cc}; {{B}}

void func(void) {
  int value = aa;
}

/*
 * check-name: enum, switched order of elements
 * assert-obj: A != B
 */
