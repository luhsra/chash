void a(){
	; {{A}}
	int a; {{B}}
}


/*
 * check-name: DeclStmt 1
 * obj-not-diff: blame optimisation
 * A != B
 */
