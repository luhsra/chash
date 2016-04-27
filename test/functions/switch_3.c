void a(){
	int b;
	switch(b){
		case 42: {{A: break;}} {{B: ;}}
		default: break;
	}
}

/*
 * check-name: case without break
 * assert-obj: A != B
 */
