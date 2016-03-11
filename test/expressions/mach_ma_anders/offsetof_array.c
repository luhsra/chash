#include <stddef.h>

/*This does only make sense in an array context*/
struct ding{
	int b;
	int array[10];
};

void func(void){
	unsigned int a = offsetof(struct ding, array[2]); {{A}}
	unsigned int a = 8; {{B}}
}
/*
 * check-name: offsetof array
 * obj-not-diff: might
 * B != A
 */
