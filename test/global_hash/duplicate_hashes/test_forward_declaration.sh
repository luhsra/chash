#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global --definition: forward declaration
# clang-hash-global will fail if there are two hash outputs for foo

check_global_hash_changed_no_copy ${0}:${LINENO} \
                          "void foo(); void bar() { foo(); } void foo() {}" \
                          "void foo(); void bar() { foo(); } void foo() {}" \
                          "foo" false \
                          "bar" false

