extern int ext; {{A}}
{{B}}
int a;

/*
 * check-name: extern variable declaration
 * assert-ast: A == B
 */
