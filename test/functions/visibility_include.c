#include "visibility/default.h"   {{A}}
#include "visibility/hidden.h"    {{B}}
#include "visibility/protected.h" {{C}}
{{D}}

int func() { return 0; }

/*
 * check-name: attribute visibility
 * obj-not-diff: A == D
 * assert-ast: A != B, A != C, A != D, B != C, B != D, C != D
 * assert-obj: A != B, A != C, A == D, B != C, B != D, C != D
 */
