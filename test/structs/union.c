struct foobar{ {{A}}
union foobar{ {{B}}
	int gelb;
	char gruen;
	unsigned long flieder_ist_keine_farbe_sondern_eine_pflanze;
};

struct foobar foo; {{A}}
union foobar foo; {{B}}

/*
 * check-name: Union or struct
 * assert-obj: B != A
 */

