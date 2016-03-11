void f1(){
	f1(); {{A}}
	{{B}}
}
/*
 * check-name: recursion
 * B != A
 */

