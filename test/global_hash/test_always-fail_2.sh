#!/bin/bash

source "/home/cip/2015/yb90ifym/clang-hash/test/global_hash/global_hash.sh"

# check-name: clang-hash-global failing testcase #2: false negative (function)
# check-known-to-fail: true

out=$(check_global_hash_changed ${0}:${LINENO} \
                          "int main_f2() {return 0;}" \
                          "int main_f2() {return 1;}" \
                          "main_f2" false)
