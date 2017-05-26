#!/bin/bash
set -e

fn=test_use_global # placeholder

DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
WORK_DIR=""

CLANG_HASH_COLLECT="${DIR}/../../build/wrappers/clang-hash-collect"
CLANG_HASH_GLOBAL="${DIR}/../../clang-hash-global"

do_copy=true
function prepare() {
    WORK_DIR=`mktemp -d -p "$DIR"`

    if [[ ! "$WORK_DIR" || ! -d "$WORK_DIR" ]]; then
        echo "Could not create temp dir"
        exit 1
    fi
    # Only the .info file is required
    if [ "$do_copy" == true ]; then
        find "${DIR}/src/" -name \*.info -exec cp {} $WORK_DIR \;
    fi

    cd $WORK_DIR
}


function cleanup() {
    rm -f ${fn}.*
}

function cleanup_all() {
    cleanup

    if [[ ! "$WORK_DIR" || ! -d "$WORK_DIR" ]]; then
    #    echo "Could not remove temp dir ${WORK_DIR}"
        echo "" # TODO
    else
        rm -rf "$WORK_DIR"
    fi
}
trap cleanup_all EXIT


function compile() {
    src="$1"; shift
    obj="$1"; shift

    env CLANG_HASH_OUTPUT_DIR=$PWD $CLANG_HASH_COLLECT -hash-verbose -c ${src} -o ${obj} 2> /dev/null
}


function recompile() {
    src="$1"; shift
    echo "${src}" > ${fn}.var.c 

    compile "${fn}.var.c" "${fn}.var.o"
}


function recompile_src() {
    # cleanup main - only for small project test, but not actually required,
    # as the .o and .o.info files are also checked in
    rm -f "main.o" "main.o.info"
    compile "main.c" "main.o"
}


function get_global_hash() {
    symbol="$1"; shift

    $CLANG_HASH_GLOBAL --definition $symbol
}


function get_global_hashes() {
    obj_file="$1"; shift

    $CLANG_HASH_GLOBAL --object-file $obj_file
}

function get_local_hash() {
    symbol="$1"; shift

    $CLANG_HASH_GLOBAL --local $symbol
}

# for checking --local
# TODO: refactor this!
function check_local_hash_changed() {
    loc="$1"; shift
    src_a="$1"; shift
    src_b="$1"; shift

    do_copy=false
    prepare

    fn="${loc/:/.}" # provide each test with a unique filename
                    # to prevent failing tests because of race conditions
    fname=$(basename $fn)
    fn="./${fname}"


    cleanup

    # recompile_src

    re_a=$(recompile "$src_a")

    #TODO: use associative arrays

    index=0
    for symbol in "$@"
    do
        if [ $(($index%2)) -eq 0 ]; then
            global_hashes_a[$index/2]=$(get_local_hash ${symbol})
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
            global_hashes_b[$index/2]=$(get_local_hash ${symbol})
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
                cleanup_all
                exit 1 # TODO: move test cases to extra files
            fi
        fi
        ((index = index + 1))
    done
   
    cleanup_all
    do_copy=true
    echo "  OK: ${loc}"
}

# for checking --definition
function check_global_hash_changed() {
    loc="$1"; shift
    src_a="$1"; shift
    src_b="$1"; shift

    prepare

    fn="${loc/:/.}" # provide each test with a unique filename
                    # to prevent failing tests because of race conditions
    fname=$(basename $fn)
    fn="./${fname}"


    cleanup

    # recompile_src

    re_a=$(recompile "$src_a")

    #TODO: use associative arrays

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
                cleanup_all
                exit 1 # TODO: move test cases to extra files
            fi
        fi
        ((index = index + 1))
    done
   
    cleanup_all
    echo "  OK: ${loc}"
}


# for checking --definition without copying the default .info file
function check_global_hash_changed_no_copy() {
    do_copy=false
    ret=$(check_global_hash_changed "$@")
    do_copy=true
    echo "$ret"
}


# for checking --object-file
function check_global_hashes_changed() {
    loc="$1"; shift
    src_a="$1"; shift
    src_b="$1"; shift

    prepare

    fn="${loc/:/.}" # provide each test with a unique filename
                    # to prevent failing tests because of race conditions
    fname=$(basename $fn)
    fn="./${fname}"


    cleanup

    # recompile_src

    re_a=$(recompile "$src_a")

    # get all hashes of the obj file and store them
    hashes=$(get_global_hashes "main.o")

    # split up the output of get_global_hashes and store the key-value pairs in a dictionary
    declare -A global_hashes_a
    while IFS=',' read -ra arr; do
        for i in "${arr[@]}"; do
            if [[ $i == *":"* ]]; then
                key=$(PYTHON_ARG="$i" python -c "import os; print os.environ['PYTHON_ARG'].split(':')[0][1:-1]")
                val=$(PYTHON_ARG="$i" python -c "import os; print os.environ['PYTHON_ARG'].split(':')[1][1:-1]")
                global_hashes_a[$key]="$val"


                # Additional check: check if --object-file and --definition provide the same hashes
                if [[ $(get_global_hash ${key}) != "$value" ]]; then
                    echo "!!!Failure ${loc}: hash of definition mode differs"
                fi
            fi
        done
    done <<< "$hashes"


    cleanup

    re_b=$(recompile "$src_b")

    # get all hashes of the obj file and store them
    hashes=$(get_global_hashes "main.o")

    # split up the output of get_global_hashes and store the key-value pairs in a dictionary
    declare -A global_hashes_b
    while IFS=',' read -ra arr; do
        for i in "${arr[@]}"; do
            if [[ $i == *":"* ]]; then
                key=$(PYTHON_ARG="$i" python -c "import os; print os.environ['PYTHON_ARG'].split(':')[0][1:-1]")
                val=$(PYTHON_ARG="$i" python -c "import os; print os.environ['PYTHON_ARG'].split(':')[1][1:-1]")
                global_hashes_b[$key]="$val"

                # Additional check: check if --object-file and --definition provide the same hashes
                if [[ $(get_global_hash ${key}) != "$value" ]]; then
                    echo "!!!Failure ${loc}: hash of definition mode differs"
                fi
             fi
        done
    done <<< "$hashes"


    # get the expected changes from the arguments
    declare -A expected
    index=0
    for symbol in "$@"; do
        if [ $(($index%2)) -eq 0 ]; then
            key="$symbol"
        else
            expected["$key"]="$symbol"
        fi
        ((index = index + 1))
    done


    # check if the expectations are met
    for symbol in ${!expected[@]}; do
        [[ ${global_hashes_a["$symbol"]} == ${global_hashes_b["$symbol"]} ]] \
            && hashes_differ=false \
            || hashes_differ=true

        if [ $hashes_differ != "${expected["$symbol"]}" ]; then
            if [ $hashes_differ = true ]; then
                echo "!!!Failure ${loc}: hashes differ, should be the same!"
            else
                echo "!!!Failure ${loc}: hashes are the same, should differ!"
            fi
            cleanup_all
            exit 1
        fi
    done


    cleanup_all
    echo "  OK: ${loc}"
}
