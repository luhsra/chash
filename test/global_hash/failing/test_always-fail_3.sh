#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global failing testcase #3: false positive (variable)
# check-known-to-fail: true

out=$(check_global_hash_changed ${0}:${LINENO} \
                          "int global_f3=0;" \
                          "int global_f3=0;" \
                          "global_f3" true)
