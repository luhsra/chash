#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global --object-file; failing
# check-known-to-fail: true

out=$(check_global_hash_changed ${0}:${LINENO} \
                          "const int i=3; int foo() {return i;}" \
                          "const int i=4; int foo() {return i;}" \
                          "main" false \
                          "changed" false \
                          "unchanged" false)

