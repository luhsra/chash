void a(){
	int b;
	switch(b){			{{B}}
		case 0: break;		{{B}}
		case 34: break;		{{B}}
		default: break;		{{B}}
	}				{{B}}
	;	{{A}}
}

/*
 * check-name: switch-Stmt
 */
