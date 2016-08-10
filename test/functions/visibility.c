int
__attribute__ ((visibility ("default")))   {{A}}
__attribute__ ((visibility ("hidden")))    {{B}}
__attribute__ ((visibility ("protected"))) {{C}}
{{D}}
func () {
  return 0;
}

/*
 * check-name: attribute visibility
 * obj-not-diff: A == D
 * assert-ast: A != B, A != C, A != D, B != C, B != D, C != D
 * assert-obj: A != B, A != C, A == D, B != C, B != D, C != D
 */
