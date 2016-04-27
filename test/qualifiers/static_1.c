int first; {{A}}
static int first; {{B}}

/*
 * check-name: Testing static (not in function)
 * assert-obj: B != A
 */

