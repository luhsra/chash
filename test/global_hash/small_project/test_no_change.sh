#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global small project; no change

out=$(check_global_hash_changed ${0}:${LINENO} \
                          "const int i=3; int foo() {return i;}" \
                          "const int i=3; int foo() {return i;}" \
                          "i" false \
                          "foo" false \
                          "main" false \
                          "changed" false \
                          "unchanged" false)

