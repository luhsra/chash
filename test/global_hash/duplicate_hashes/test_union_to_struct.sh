#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global --definition: union to struct
# clang-hash-global will fail if there are two hash outputs for struct B

check_global_hash_changed_no_copy ${0}:${LINENO} \
                          "struct A { int i; double d; }; struct A a;" \
                          "union A { int i; double d; }; struct A a;" \
                          "a" true

