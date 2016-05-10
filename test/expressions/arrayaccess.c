int inty[2][4] = {{1, 2, 3, 4}, {0, 0, 0, 0}}; {{A}}
int inty[2][4] = {{1, 2, 3, 0}, {0, 0, 0, 0}}; {{B}}

int f() {
  return inty[0][1];
}

/*
 * check-name: Initlist array access
 * assert-obj: A != B
 */
