#!/bin/bash

source "/home/cip/2015/yb90ifym/clang-hash/test/global_hash/global_hash.sh"

# check-name: clang-hash-global small project 2; change used function

out=$(check_global_hash_changed ${0}:${LINENO} \
                          "const int i=3; int foo() {return i;}" \
                          "const int i=3; int foo() {return i+5;}" \
                          "i" false \
                          "foo" true \
                          "main" true \
                          "changed" false \
                          "unchanged" false)

