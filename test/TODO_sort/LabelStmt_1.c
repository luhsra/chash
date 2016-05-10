void a() {
label1: ; {{A}}
label2: ; {{B}}
        ; {{C}}
}

/*
 * check-name: empty label, not used
 * obj-not-diff: A == B
 * assert-ast: A != B
 * assert-obj: A == B, A != C, B != C
 */
