void a() {
  int b;
  switch (b) {
    case 42: break;
    default: {{A: break;}} {{B: ;}}
  }
}

/*
 * check-name: default without break
 * obj-not-diff: yes
 * assert-ast: A != B
 * assert-obj: A == B
 */
