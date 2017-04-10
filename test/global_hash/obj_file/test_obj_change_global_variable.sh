#!/bin/bash

source "/home/cip/2015/yb90ifym/clang-hash/test/global_hash/global_hash.sh"

# check-name: clang-hash-global --object-file: change used function

check_global_hashes_changed ${0}:${LINENO} \
                          "const int i=3; int foo() {return i;}" \
                          "const int i=4; int foo() {return i;}" \
                          "main" true \
                          "changed" true \
                          "unchanged" false

