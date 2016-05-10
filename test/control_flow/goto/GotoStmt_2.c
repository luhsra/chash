void a() {
ende: ;
  ;          {{A}}
  goto ende; {{B}}
}

/*
 * check-name: Goto backwards
 * assert-obj: A != B
 */
