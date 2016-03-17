void func(void); {{A}}
{{B}}
int main(){
	func(); {{A}}
	return 0;
}

/*
 * check-name: void FuncDecl used
 * B != A
 */
