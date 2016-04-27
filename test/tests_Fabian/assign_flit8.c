float a = 1.0;	{{A}}
float a = 1.0f;	{{B}}
float a = 1.0F;	{{C}}


/*
 * check-name: same value but different float literals
 * obj-not-diff: because 1.0 is double...
 * assert-ast: A != B, B == C, C != A
 * assert-obj: A == B, B == C, C == A
 */
