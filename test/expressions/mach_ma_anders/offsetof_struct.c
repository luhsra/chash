struct test{
	int a1;
	int a2;
}

size_t a = offsetof(test, a2); {{A}}
size_t a = 4; {{B}}

/*
 * check-name: offsetof struct
 * obj-not-diff: might
 * B != A
 */
