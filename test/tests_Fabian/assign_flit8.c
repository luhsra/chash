float a = 1.0;	{{A}}
float a = 1.0f;	{{B}}
float a = 1.0F;	{{C}}


/*
 * check-name: same value but different float literals
 * A == B == C
 * obj-not-diff: because 1.0 is double...
 */
