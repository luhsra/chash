#!/bin/bash
set -e

# check-name: Abort compilation on second invocation

function cleanup() {
    rm -f test_abort.c test_abort.o*
}
trap cleanup EXIT

function recompile() {
    src="$1"; shift

    echo "${src}" > test_abort.c
    skipped=false
    clang-hash-stop -Xclang -plugin-arg-clang-hash -Xclang -hash-verbose -c test_abort.c \
                    2>&1 >/dev/null |  grep -q '^skipped: *1'

    if [ $? -eq 0 ]; then
        echo "true";
    else
        echo "false"
    fi
}

function check_is_recompiled() {
    loc="$1"; shift
    expected="$1"; shift
    src_a="$1"; shift
    src_b="$1"; shift

    cleanup
    re_a=$(recompile "$src_a")
    re_b=$(recompile "$src_b")

    if [ ${re_a} = "true" ]; then
        echo "!!!Failure ${loc}: initial compilation was skipped"
        exit 1
    fi
    if [ ${re_b} != ${expected} ]; then
        echo "!!!Failure ${loc}: wrong skip status (${re_b})"
        echo "${src_a} -> ${src_b}"
        exit 1
    fi

    echo "  OK: ${loc} skipped=${re_b}"
}

# Not skipped checks
check_is_recompiled ${0}:${LINENO} false \
                    "int main() {return 0;}" \
                    "int main() {return 1;}"

# Skipped checks
check_is_recompiled ${0}:${LINENO} true \
                    "int main() {return 0;}" \
                    "typedef int foo; int main() {return 0;}"
