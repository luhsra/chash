void f(){

	static int a; {{A}}
	int a; {{B}}
//pay attention wheter obj-file is even created:
// the next line enforces this!
	a = 3;
}




/*
 * obj-not-diff: yes
 * check-name: Testing static (in function)
 * B != A
 */
