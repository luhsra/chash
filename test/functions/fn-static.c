static int foo() {
    return 123; {{A}}
    return 453; {{B}}
}

/*
 * check-name: static function
  * assert-ast: A != B
  * obj-not-diff: maybe (if `foo' is optimized out)
  */
