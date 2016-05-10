int first;        {{A}}
static int first; {{B}}

/*
 * check-name: global static variable
 * assert-obj: A != B
 */

