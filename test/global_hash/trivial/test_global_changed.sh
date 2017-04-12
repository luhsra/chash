#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global change of used global variable

out=$(check_global_hash_changed ${0}:${LINENO} \
                          "int global_c=0; int main_c() {return global_c;}" \
                          "int global_c=1; int main_c() {return global_c;}" \
                          "main_c" true \
                          "global_c" true)

