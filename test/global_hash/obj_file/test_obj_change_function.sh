#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global --object-file: change used function

check_global_hashes_changed ${0}:${LINENO} \
                          "const int i=3; int foo() {return i;}" \
                          "const int i=3; int foo() {return i+5;}" \
                          "main" true \
                          "changed" false \
                          "unchanged" false

