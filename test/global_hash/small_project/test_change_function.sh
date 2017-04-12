#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global small project; change used function

out=$(check_global_hash_changed ${0}:${LINENO} \
                          "const int i=3; int foo() {return i;}" \
                          "const int i=3; int foo() {return i+5;}" \
                          "i" false \
                          "foo" true \
                          "main" true \
                          "changed" false \
                          "unchanged" false)

