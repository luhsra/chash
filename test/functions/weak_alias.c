#define weak_alias(old, new) \
    extern __typeof(old) new __attribute__((weak, alias(#old)))

int func () {
  return 0;
}

{{A}}
weak_alias(func, func2); {{B}}
weak_alias(func, func3); {{C}}

/*
 * check-name: attribute weak_alias
 * assert-ast: A != B, A != C, B != C
 */

