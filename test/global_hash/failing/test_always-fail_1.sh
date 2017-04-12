#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global failing testcase #1: false positive (function)
# check-known-to-fail: true

out=$(check_global_hash_changed ${0}:${LINENO} \
                          "int main_f1() {return 0;}" \
                          "int main_f1() {return 0;}" \
                          "main_f1" true)
