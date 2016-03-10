void f1(){
}
void f2(){
	{{A}}
	f1(); {{B}}
}
/*
 * check-name: CallExpr
 * obj-not-diff: ?
 * B != A
 */

