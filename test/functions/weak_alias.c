
int
__attribute__((weak)) {{A}}
{{B}}
func () {
  return 0;
}


/*
 * check-name: attribute weak_alias
 * assert-ast: A != B
 * assert-obj: A != B
 */

