void f() {
  int a = ({int x = 7; {{A:++x;}} {{B:x++;}} {{C:8;}}});
}

/*
 * check-name: STMTEXPR
 * assert-obj: A != B, A != C, B != C
 */
