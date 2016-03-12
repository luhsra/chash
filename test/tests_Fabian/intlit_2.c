int a = 42;		{{A}}
int a = 052;	{{B}}
int a = 0x2a;	{{C}}
int a = 0x2A;	{{D}}


/*
 * check-name: same value but different integer literal 2
 * A == B == C == D
 */
