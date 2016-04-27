void func(void); {{A}}
{{B}}
int main(){
	return 0;
}

/*
 * check-name: void FuncDecl unused
 * assert-obj: A == B
 */
