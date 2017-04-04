#!/bin/bash

source "/home/cip/2015/yb90ifym/clang-hash/test/global_hash/global_hash.sh"

# check-name: clang-hash-global small project; change value of a used global variable

out=$(check_global_hash_changed ${0}:${LINENO} \
                          "const int i=3; int foo() {return i;}" \
                          "const int i=4; int foo() {return i;}" \
                          "i" true \
                          "foo" true \
                          "main" true \
                          "changed" true \
                          "unchanged" false)

