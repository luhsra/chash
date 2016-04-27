void a(void)
{
	int a;
	struct b{
		int a;
		int b;
	};
	unsigned int s = sizeof(struct b); {{A}}

	unsigned int s = sizeof(int); {{B}}

	unsigned int s = sizeof(a); {{C}}
}

/*
 * check-name: sizeof
 * obj-not-diff: B==C
 * assert-ast: B != A, B != C, C != A
 * assert-obj: A != B, B == C, C != A
 */
