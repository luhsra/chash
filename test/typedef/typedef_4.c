typedef const int used;

used first; {{A}}
int first;  {{B}}

/*
 * check-name: typedef const
 * obj-not-diff: yes
 * -ast A != B
 * assert-obj: A == B
 */
//TODO: BUG?! beim testen ist ast ==, warum ist ast nicht !=???
// wenn ich used mit const int ersetze ists unterschiedlich
