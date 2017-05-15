#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global --definition: incomplete type
# clang-hash-global will fail if there are two hash outputs for struct B

check_global_hash_changed_no_copy ${0}:${LINENO} \
                          "struct A { struct B *p; }; struct B { int i; }; struct B b; struct A a;" \
                          "struct A { struct B *p; }; struct B { float i; }; struct B b; struct A a;" \
                          "a" false \
                          "b" true

