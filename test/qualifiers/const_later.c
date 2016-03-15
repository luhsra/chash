int first; {{A}}
const int first; {{B}}

int wombat;
const int wombatino;
/*
 * check-name: Testing const then without
 * obj-not-diff: yes
 * B != A
 */

