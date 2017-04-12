#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global simple change #4 (variable)

out=$(check_global_hash_changed ${0}:${LINENO} \
                          "int global_s4=0;" \
                          "int global_s4=0;" \
                          "global_s4" false)
