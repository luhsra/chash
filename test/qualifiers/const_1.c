int first; {{A}}
const int first; {{B}}

char second;
/*
 * check-name: Testing const
 * obj-not-diff: yes
 * B != A
 */

