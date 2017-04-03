#!/bin/bash

#this=$(readlink -f "${BASH_SOURCE[0]}" 2>/dev/null||echo $0)
#dir=$(dirname "${this}")
#. "$dir/global_hash.sh"
source "/home/cip/2015/yb90ifym/clang-hash/test/global_hash/global_hash.sh"

# check-name: clang-hash-global failing testcase #1: false positive (function)
# check-known-to-fail: true

out=$(check_global_hash_changed ${0}:${LINENO} \
                          "int main_f1() {return 0;}" \
                          "int main_f1() {return 0;}" \
                          "main_f1" true)
