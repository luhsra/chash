int a = 1?2:0; {{A}}
int a = 0?2:0; {{B}} 

/*
 * check-name: ConditionOperator
 * assert-obj: A != B
 */
