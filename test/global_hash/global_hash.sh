#!/bin/bash
set -e

fn=test_use_global # placeholder

# TODO: make paths independent


function cleanup() {
    rm -f ${fn}.*
}
trap cleanup EXIT


function compile() {
    src="$1"; shift
    obj="$1"; shift

    env CLANG_HASH_OUTPUT_DIR=$PWD "/home/cip/2015/yb90ifym/clang-hash/build/wrappers/clang-hash-collect" -hash-verbose -c ${src} -o ${obj} 2> /dev/null
}


function recompile() {
    src="$1"; shift
    echo "${src}" > ${fn}.var.c 

    compile "${fn}.var.c" "${fn}.var.o"
}


function get_global_hash() {
    symbol="$1"; shift

    "/home/cip/2015/yb90ifym/clang-hash/clang-hash-global" --definition $symbol
}


function check_global_hash_changed() { # checks for a single symbol
    loc="$1"; shift
    src_a="$1"; shift
    src_b="$1"; shift

    fn="${loc/:/.}" # provide each test with a unique filename
                    # to prevent failing tests because of race conditions

    cleanup

    # cleanup main - only for small project test, but not actually required,
    # as the .o and .o.info files are also checked in
    #rm -f "main.o" "main.o.info"
    #compile "main.c" "main.o"

    re_a=$(recompile "$src_a")

    index=0
    for symbol in "$@"
    do
        if [ $(($index%2)) -eq 0 ]; then
            global_hashes_a[$index/2]=$(get_global_hash ${symbol})
        else
            expected[$index/2]=$symbol
        fi
        
        ((index = index + 1))
    done
    

    cleanup
 
    re_b=$(recompile "$src_b")

    index=0
    for symbol in "$@"
    do
        if [ $(($index%2)) -eq 0 ]; then
            global_hashes_b[$index/2]=$(get_global_hash ${symbol})
        fi
        ((index = index + 1))
    done


    index=0
    for symbol in "$@"
    do
        if [ $(($index%2)) -eq 0 ]; then
            [[ ${global_hashes_a[$index/2]} == ${global_hashes_b[$index/2]} ]] \
                && hashes_differ=false \
                || hashes_differ=true

            if [ $hashes_differ != ${expected[$index/2]} ]; then
                if [ $hashes_differ = true ]; then
                    echo "!!!Failure ${loc}: hashes differ, should be the same!"
                else
                    echo "!!!Failure ${loc}: hashes are the same, should differ!"
                fi
                cleanup
                exit 1 # TODO: move test cases to extra files
            fi
        fi
        ((index = index + 1))
    done
   
    cleanup
    echo "  OK: ${loc}"
}

