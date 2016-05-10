int s = 25;

int func() {
  int a[5]; {{A}}
  int a[s/5]; {{B}}
}

/*
 * check-name: array size from var
 * assert-obj: A != B
 */
