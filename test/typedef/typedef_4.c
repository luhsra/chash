typedef const int used;

used first; {{A}}
int first; {{B}}
/*
 * check-name: typedef const
 * obj-not-diff: yes
 * A != B
 */
//TODO: beim testen sind ast und obj beide ==
