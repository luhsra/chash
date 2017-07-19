
struct { int i; } u;     {{A}}

struct Named { int i; }; {{B}}
struct Named u;          {{B}}

/*
 * check-name: unnamed struct
 * assert-ast: A == B
 */


