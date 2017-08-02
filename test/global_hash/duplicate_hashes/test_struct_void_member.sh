#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global --local: function call changes unrelated hash of a struct

check_local_hash_changed ${0}:${LINENO} \
                          "struct S { void *s; }; void foo (void *p); void func() { (void) sizeof(struct S); }" \
                          "struct S { void *s; }; void foo (void *p); void func() { foo(0); (void) sizeof(struct S); }" \
                          "S:test_struct_void_member.sh.13.var.c" false \
                          "func" true

