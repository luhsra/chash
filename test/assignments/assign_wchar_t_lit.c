#include <wchar.h>
#include <stdlib.h>

void fun() {
  char *from = "char1"; {{A}}
  char *from = "char2"; {{B}}
  wchar_t *c = NULL;
    
  mbstowcs(c, from, 6);
}

/*
 * check-name: wide character string literal
 * assert-obj: A != B 
 */
