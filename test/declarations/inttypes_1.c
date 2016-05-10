int a = 4711;        {{A}}
signed int a = 4711; {{B}}
signed a = 4711;     {{C}}

/*
 * check-name: same types but different name
 * assert-ast: A == B, A == C, B == C
 */
