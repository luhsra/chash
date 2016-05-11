#include <stdio.h>

int main(void)
{
  int foo = 5;
  int bar = 4;

  __asm__ (
      "add %1, %0\n\t" {{A}}
{{B}}
      "inc %0"
      : "+r" (bar)
      : "g" (foo)
      : "cc"
  );

  printf("Ergebnis: %i\n", bar);
  return 0;
}

/*
 * check-name: inline assembler
 * assert-obj: A != B
 */

