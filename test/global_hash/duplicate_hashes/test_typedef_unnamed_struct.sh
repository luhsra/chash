#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global --definition: typedef'ed unnamed struct
# clang-hash-global will fail if there are two hash outputs for records without a name

check_global_hash_changed_no_copy ${0}:${LINENO} \
                          "typedef struct {float i;} A; typedef struct {int i;} B; A a; B b;" \
                          "typedef struct {double i;} A; typedef struct {int i;} B; A a; B b;" \
                          "a" true \
                          "b" false
