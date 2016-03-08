int first; {{A}}
volatile int first; {{B}}

/*
 * check-name: Testing volatile
 * obj-not-diff: yes
 * B != A
 */

