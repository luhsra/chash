long a = 4711;	{{A}}
long a = 4711l;	{{B}}
long a = 4711L;	{{C}}

/*
 * check-name: same value but different integer literal 1
 * A == B == C
 * fails because 4711 is int...
 */
