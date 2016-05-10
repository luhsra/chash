int a = 42;   {{A}}
int a = 052;  {{B}}
int a = 0x2a; {{C}}
int a = 0x2A; {{D}}

/*
 * check-name: same value but different integer literal 3 (dec vs oct vs hex)
 * assert-ast: A == B, A == C, A == D, B == C, B == D, C == D
 */
