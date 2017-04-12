#!/bin/bash

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

source "$DIR/../global_hash.sh"

# check-name: clang-hash-global no change

out=$(check_global_hash_changed ${0}:${LINENO} \
                          "int main_s1() {return 0;}" \
                          "int main_s1() {return 0;}" \
                          "main_s1" false)
