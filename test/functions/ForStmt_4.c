void a(){
	int b = 0;
    for(int i = 0;
        {{A: i < 100}}
        {{B: b < 100}}
        {{C: i}}
        {{D: b}}
        {{E:  }}
        ; i++){}
}

/*
 * check-name: condStmt in for
 * all versions should differ
 * assert-obj: A != B, A != C, A != D, A != E, B != C, B != D, B != E, C != D, C != E, D != E
 */
