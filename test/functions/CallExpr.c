void f1() {
}

void f2() {
        {{A}}
  f1(); {{B}}
}

/*
 * check-name: CallExpr
 * assert-obj: A != B
 */
