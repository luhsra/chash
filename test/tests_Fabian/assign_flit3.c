double a = 1.0;		{{A}}
double a = 1.0f;	{{B}}
double a = 1.0F;	{{C}}


/*
 * check-name: same value but different float literals
 * A == B == C
 * fails because *f has to be casted to double
 */
