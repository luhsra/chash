void a() {
  while (0) {}         {{A}}
  while (0) { break; } {{B}}
}

/*
 * check-name: break in while (0)
 * assert-obj: A != B
 */
