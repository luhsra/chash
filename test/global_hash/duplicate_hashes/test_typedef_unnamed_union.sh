#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global --definition: typedef'ed unnamed union
# clang-hash-global will fail if there are two hash outputs for records without a name

check_global_hash_changed_no_copy ${0}:${LINENO} \
                          "typedef union {float i;} A; typedef union {int i;} B; A a; B b;" \
                          "typedef union {double i;} A; typedef union {int i;} B; A a; B b;" \
                          "a" true \
                          "b" false
