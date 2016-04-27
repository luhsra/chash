void f(){
	int a = ({int x = 7; ++x;}); {{A}}
	int a = ({int x = 7; x++;}); {{B}}
}

/*
 * check-name: STMTEXPR
 * assert-obj: A != B
 */
