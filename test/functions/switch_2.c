void a(){
	int b;
	switch(b){
		case 0: break;
		case 42: break; {{A}}
		default: break; {{B}}
	}
}

/*
 * check-name: switch without default
 * A != B
 */
