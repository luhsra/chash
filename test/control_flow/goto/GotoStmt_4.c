void a() {
  int b;
  b = 0;      {{A}}
ende:  b = 0; {{B}}
  goto ende;  {{B}}
}

/*
 * check-name: goto backwards to not empty label
 * assert-obj: A != B
 */
