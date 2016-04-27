
//This tests whether useless structs are ignored!

struct not_used{ {{A}}
	int a; {{A}}
	int b; {{A}}
}; {{A}}

{{B}}
struct used_with_typedef{
	int c;
	int d;
};

struct used_without_typedef{
	int e;
	int f;
};

typedef struct used_with_typedef used;

int func(void){
	
	used used1;
	struct used_without_typedef used2;
	return 0;
}
/*
 * check-name: struct 4
 * obj-not-diff: might be the same size
 * assert-ast: A == B
 * assert-obj: A == B
 */
//TODO: why is ast also == ? seems not_used is completely ignored
