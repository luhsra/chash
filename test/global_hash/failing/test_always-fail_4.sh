#!/bin/bash

source "/home/cip/2015/yb90ifym/clang-hash/test/global_hash/global_hash.sh"

# check-name: clang-hash-global failing testcase #4: false negative (variable)
# check-known-to-fail: true

out=$(check_global_hash_changed ${0}:${LINENO} \
                          "int global_f4=0;" \
                          "int global_f4=1;" \
                          "global_f4" false)

