#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global --definition: unnamed struct
# clang-hash-global will fail if there are two hash outputs for records without a name

check_global_hash_changed_no_copy ${0}:${LINENO} \
                          "struct {int i;} u;" \
                          "struct Named {int i;}; struct Named u;" \
                          "u" false

