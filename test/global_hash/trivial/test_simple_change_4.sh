#!/bin/bash

source "/home/cip/2015/yb90ifym/clang-hash/test/global_hash/global_hash.sh"

# check-name: clang-hash-global simple change #4 (variable)

out=$(check_global_hash_changed ${0}:${LINENO} \
                          "int global_s4=0;" \
                          "int global_s4=0;" \
                          "global_s4" false)
