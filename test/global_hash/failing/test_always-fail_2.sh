#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global failing testcase #2: false negative (function)
# check-known-to-fail: true

out=$(check_global_hash_changed ${0}:${LINENO} \
                          "int main_f2() {return 0;}" \
                          "int main_f2() {return 1;}" \
                          "main_f2" false)
