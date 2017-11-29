
struct { int i; } u;     {{A}}

struct Named { int i; }; {{B}}
struct Named u;          {{B}}

/*
 * check-name: Names of structs matter
 * obj-not-diff: maybe
 * assert-ast: A != B
 */


