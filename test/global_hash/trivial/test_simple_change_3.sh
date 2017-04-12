#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global simple change #3 (variable)

out=$(check_global_hash_changed ${0}:${LINENO} \
                          "int global_s3=0;" \
                          "int global_s3=1;" \
                          "global_s3" true)
