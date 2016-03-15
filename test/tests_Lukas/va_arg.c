#include <stdarg.h>



static int vrdval(va_list arg_ptr)
{
	int indexnext;
	indexnext = va_arg(arg_ptr, int);
/*
	while (1) {ma
		while (*p == ' ' || *p == '\t') p++;
		if (*p == '\n' || *p == '\0') break;

		if (indexline == indexnext) { // read this value
			*vec++ = conv==conv_decimal ?
				strtoull(p, NULL, 10) :
				read_after_slash(p);
			indexnext = va_arg(arg_ptr, int);
		}
		while (*p > ' ') p++; // skip over value
		indexline++;
	}
*/
	return 0;
}

{{A}}

/*
 * check-name: va_arg found problematic in bb-project
 */
