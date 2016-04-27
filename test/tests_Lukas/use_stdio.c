#include <stdio.h>

int main(){


	printf("Hello World\n"); {{A}}
	printf("Ciao World\n"); {{B}}

}
/*
 * check-name: use stdio.h
 * assert-obj: A != B
 */
