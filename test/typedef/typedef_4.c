typedef const int used;

used first; {{A}}
int first; {{B}}
/*
 * check-name: typedef const
 * obj-not-diff: yes
 * B != A
 */

