void f1(){
	f1(); {{A}}
	{{B}}
}
/*
 * check-name: recursion
 * assert-obj: B != A
 */

