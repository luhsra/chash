
int f(void){
	long a = 1; {{A}}
	long long a = 1; {{B}}
	return a;
}
/*
 * check-name: struct 1
 * obj-not-diff: reasonabale
 * B != A
 */

