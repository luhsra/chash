#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global --object-file: no change

check_global_hashes_changed ${0}:${LINENO} \
                          "const int i=3; int foo() {return i;}" \
                          "const int i=3; int foo() {return i;}" \
                          "main" false \
                          "changed" false \
                          "unchanged" false

