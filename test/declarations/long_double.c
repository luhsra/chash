long double var = 1.0L; {{A}}
long double var = 1.0;  {{B}}
double var = 1.0;       {{C}}
long long var = 1.0;    {{D}}

/*
 * check-name: long double
 * obj-not-diff: A == B
 * assert-ast: A != B
 * assert-obj: A == B, A != C, A != D, B != C, B != D, C != D
 */
