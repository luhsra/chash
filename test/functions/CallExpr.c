void f1(){
}
void f2(){
	{{A}}
	f1(); {{B}}
}
/*
 * check-name: CallExpr
 * obj-not-diff: ? seem to differ slightly
 * assert-ast: A != B
 * assert-obj: A != B
 */
