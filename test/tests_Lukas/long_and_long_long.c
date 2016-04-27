
int f(void){
	long a = 1; {{A}}
	long long a = 1; {{B}}
	return a;
}
/*
 * check-name: struct 1
 * obj-not-diff: reasonable
 * assert-ast: A != B
 * assert-obj: A == B
 */
//TODO: obj==/ast!= ok so? long === long long ?
