#include <stdio.h>


void f(){
	{{A}}
	{{B}}
}


/*
 * check-name: Definition of unused enum
 * assert-obj: A == B
 */
