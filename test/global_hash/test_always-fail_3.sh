#!/bin/bash

source "/home/cip/2015/yb90ifym/clang-hash/test/global_hash/global_hash.sh"

# check-name: clang-hash-global failing testcase #3: false positive (variable)
# check-known-to-fail: true

out=$(check_global_hash_changed ${0}:${LINENO} \
                          "int global_f3=0;" \
                          "int global_f3=0;" \
                          "global_f3" true)
