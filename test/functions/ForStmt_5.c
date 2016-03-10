void a(){
	int b = 0;
	for(int i = 0; i < 100; {{A: i++}} {{B: b++}} {{C: i}} {{D: b}} {{E:  }}){}
}

/*
 * check-name: incStmt on for
 * all versions should differ from each other
 */
