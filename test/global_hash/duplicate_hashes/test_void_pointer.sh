#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global --definition: unused variable definition changes unrelated hash
# clang-hash-global will fail if there are two hash outputs for foo

check_global_hash_changed_no_copy ${0}:${LINENO} \
                          "int i; void foo() { int j; }" \
                          "float i; void foo() { int j; }" \
                          "i" true \
                          "foo" false

