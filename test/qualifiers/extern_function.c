extern int ext(void); {{A}}
{{B}}
int a;

/*
 * check-name: extern function declaration
 * assert-ast: A == B
 */
