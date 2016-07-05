extern int ext(void); {{A}}
{{B}}

/*
 * check-name: extern function declaration
 * assert-ast: A == B
 */
