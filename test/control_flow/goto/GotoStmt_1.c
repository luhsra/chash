void a() {
  ;          {{A}}
  goto ende; {{B}}
ende: ;      {{B}}
}

/*
 * check-name: Goto forwards
 * assert-obj: A != B
 */
