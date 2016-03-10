void a(){
	int b;
	switch(b){
		case 42: break; {{A}}
		default: {{B: ;}} break;
	}
}

/*
 * check-name: switch without cases
 * A != B
 */
