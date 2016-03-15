
struct foobar{
	int gelb;
};



void func(void){
	struct foobar* a =(struct cc_trie*)  (sizeof(*a));
}


{{A}}

/*
 * check-name: size of the own declaration --> Seg-Fault?
 */

