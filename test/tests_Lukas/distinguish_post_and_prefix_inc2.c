

void f(){
	int a = 3;
	a++;	{{A}}
	++a;	{{B}}
	int b = a;
	
}
	
/*
 * check-name: shoult not distinguish, when not necessarry
 * obj-not-diff:object file isn't changed indeed!
 * A == B
 */
