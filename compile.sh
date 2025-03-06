#!/bin/bash

#
function test_cmd_exist_only() {
    local TMP_CMD="$1"
    which ${TMP_CMD} >/dev/null 2>&1
    if test $? -ne 0; then
        test -n "${TMP_CMD}" && echo "${TMP_CMD} not found!!!"
    fi
}

#
function test_cmd_exist_exit() {
    local TMP_CMD="$1"
    which ${TMP_CMD} >/dev/null 2>&1
    if test $? -ne 0; then
        test -n "${TMP_CMD}" && echo "${TMP_CMD} not found!!!"
        exit 1
    fi
}

test_cmd_exist_exit bash
test_cmd_exist_exit basename
test_cmd_exist_exit dirname
test_cmd_exist_exit realpath
test_cmd_exist_exit awk
test_cmd_exist_exit tr
test_cmd_exist_exit sed
test_cmd_exist_exit grep
test_cmd_exist_exit wc
test_cmd_exist_exit head
test_cmd_exist_exit uname
test_cmd_exist_exit nproc
test_cmd_exist_exit seq
test_cmd_exist_only date

SCRIPT_DIR=$(dirname $(realpath ${BASH_SOURCE}))

#
TARGET_HOST=${TARGET_HOST=host_asan}
# toolchain env
TOOLCHAIN_ENV_FILE=${SCRIPT_DIR}/toolchain/toolchain_${TARGET_HOST}.sh
if test ! -e ${TOOLCHAIN_ENV_FILE}; then
    echo "file not exist: ${TOOLCHAIN_ENV_FILE}"
    exit 1
fi
source ${TOOLCHAIN_ENV_FILE}
${CXX} --version
if test $? -ne 0; then
    echo "Please check CXX: ${CXX}"
    exit 1
fi
${CC} --version
if test $? -ne 0; then
    echo "Please check CC: ${CC}"
    exit 1
fi
${AR} --version
if test $? -ne 0; then
    echo "Please check AR: ${AR}"
    exit 1
fi
echo ""

cd ${SCRIPT_DIR}
export SRC_DIR=${SRC_DIR=src}
export BUILD_DIR=${BUILD_DIR=build_${TARGET_HOST}}
export TARGET_NAME=${TARGET_NAME=exec}

which ${MAKE} >/dev/null 2>&1
if test $? -ne 0; then
    test -n "${MAKE}" && echo "${MAKE} not found!!!"
    #
    MAKE=make
    echo "try ${MAKE}"
fi
which ${MAKE} >/dev/null 2>&1
if test $? -ne 0; then
    test -n "${MAKE}" && echo "${MAKE} not found!!!"
    #
    MAKE=gmake
    echo "try ${MAKE}"
fi
which ${MAKE} >/dev/null 2>&1
if test $? -ne 0; then
    test -n "${MAKE}" && echo "${MAKE} not found!!!"
    echo "terminate"
    exit 1
fi

# qemu to run cross-compiled program
if test "$(uname -m)" != "${SYSTEM_PROCESSOR}"; then
    QEMU_RUN_PROGRAM="qemu-${SYSTEM_PROCESSOR}"
    which ${QEMU_RUN_PROGRAM} >/dev/null 2>&1
    if test $? -eq 0; then
        if test -z "${SYSROOT}"; then
            QEMU_RUN_CMD="${QEMU_RUN_PROGRAM}"
        else
            QEMU_RUN_CMD="${QEMU_RUN_PROGRAM} -L ${SYSROOT}"
        fi
    else
        echo "${QEMU_RUN_PROGRAM} not found!!! cannot run cross-compiled program"
    fi
fi

# sometimes it is needed to process PATH in Windows OS
function escape_path() {
    local TMPPATH="$(realpath $1)"
    if test "$(uname)" = "Linux"; then
        echo $TMPPATH
    else
        which cygpath >/dev/null
        if test $? -ne 0; then
            echo $TMPPATH
        else
            cygpath -m $TMPPATH
        fi
    fi
}
export -f escape_path

function regen_mk() {
    rm -rf ${BUILD_DIR}
    ${SCRIPT_DIR}/builder/glob_src.sh
    test $? -ne 0 && exit 1
}

# generate compile_commands.json if we have compiledb
function regen_compiledb() {
    ${MAKE} -C ${BUILD_DIR} clean >/dev/null
    test $? -ne 0 && exit 1

    ${MAKE} -C ${BUILD_DIR} -n >${BUILD_DIR}/build.log
    #
    COMPILE_DB_CMD=$(which compiledb)
    if test -n "${COMPILE_DB_CMD}"; then
        echo ""
        cd ${BUILD_DIR}
        ${COMPILE_DB_CMD} -v -S -p build.log
        cd -
        cp ${BUILD_DIR}/compile_commands.json .
        echo ""
    else
        echo "compiledb not found, it is needed for generating compile_commands.json"
        echo "consider installing it by cmd: pip install compiledb"
    fi
}

# generate .clangd
function regen_clangd_config() {
    local TMP_CC
    local TMP_CXX
    local TMP_SYSROOT_DIR="${SYSROOT}"
    local CLANGD_CONF="$1"
    if test -z "$CLANGD_CONF"; then
        CLANGD_CONF=".clangd_autogen"
    fi
    ${MAKE} -C ${BUILD_DIR} dump_compile_info

    TMP_CC=$(cat ${BUILD_DIR}/c_compiler)
    TMP_CXX=$(cat ${BUILD_DIR}/cxx_compiler)

    if test -n "$TMP_CC"; then
        TMP_COMPILER=$TMP_CC
    elif test -n "$TMP_CXX"; then
        TMP_COMPILER=$TMP_CXX
    fi

    cat ${BUILD_DIR}/cppflags | tr ' ' '\n' | awk '!seen[$0]++' >${BUILD_DIR}/cppflags_strip
    #
    if test -n "$TMP_COMPILER"; then
        #
        ${TMP_COMPILER} -E -xc++ -v /dev/null 2>&1 | sed -e 's/\\/\//g' | while read LINE; do
            TMPCNT=$(echo $LINE | grep -v '=' | awk -F'#' '{print $1}' | grep -oP '[[:graph:]]+' | wc -l)
            if test "$TMPCNT" = "1"; then
                TMPPATH=$(realpath $(echo $LINE | grep -oP '[[:graph:]]+' | head -n 1))
                # echo $TMPPATH
                if test -d "${TMPPATH}"; then
                    echo -I$(escape_path ${TMPPATH}) >>${BUILD_DIR}/cppflags_strip
                fi
            fi
        done
    fi
    #
    if test -n "$TMP_SYSROOT_DIR"; then
        TMP_SYSROOT_DIR="$(escape_path $TMP_SYSROOT_DIR)"
        echo "TMP_SYSROOT_DIR: ${TMP_SYSROOT_DIR}"
        echo "--sysroot=$TMP_SYSROOT_DIR" >>${BUILD_DIR}/cppflags_strip
    fi

    rm -f ${BUILD_DIR}/cppflags_strip_lines
    cat ${BUILD_DIR}/cppflags_strip | sed -e 's/-I/-I\n/g' | while read line; do
        (
            if test "$line" = "-I"; then
                flag="$line"
            elif test -d "$BUILD_DIR/$line"; then
                flag="$(escape_path $BUILD_DIR/$line)"
            else
                flag="$line"
            fi
            echo "$flag" >>${BUILD_DIR}/cppflags_strip_lines
        )
    done

    # https://clangd.llvm.org/config.html#diagnostics
    cat >${CLANGD_CONF} <<EOF
# auto-generated by $BASH_SOURCE
EOF

    cat >>${CLANGD_CONF} <<EOF
---
If:                               
  PathMatch: .*\.(h|H|hpp|hxx|cc|cpp|CPP|cxx)    
Diagnostics:      
  #ClangTidyChecks: true       
  ClangTidy:             
    Add: 
      - "bugprone-*"
      - "performance-*"
CompileFlags:           
  Compiler: $(which ${TMP_CXX})
  Add:
    - "-Wall"           
    - "-Wextra" 
    - "-xc++"   
EOF
    cat ${BUILD_DIR}/cppflags_strip_lines | while read line; do
        echo "    - \"$line\"" >>${CLANGD_CONF}
    done

    cat >>${CLANGD_CONF} <<EOF
---
If:                               
  PathMatch: .*\.(c|C)    
Diagnostics:      
  #ClangTidyChecks: true       
  ClangTidy:             
    Add: 
      - "bugprone-*"
      - "performance-*"
CompileFlags:           
  Compiler: $(which ${TMP_CC})
  Add:
    - "-Wall"           
    - "-Wextra"     
EOF
    cat ${BUILD_DIR}/cppflags_strip_lines | while read line; do
        echo "    - \"$line\"" >>${CLANGD_CONF}
    done
}

function clean_target() {
    ${MAKE} -C ${BUILD_DIR} clean
    test $? -ne 0 && exit 1
}

function build_target() {
    ${MAKE} -C ${BUILD_DIR} -j1
    test $? -ne 0 && exit 1
}

function rebuild_target() {
    ${MAKE} -C ${BUILD_DIR} clean
    test $? -ne 0 && exit 1
    ${MAKE} -C ${BUILD_DIR} -j$(nproc)
    test $? -ne 0 && exit 1
}

function run_target() {
    echo ""
    CMD="${QEMU_RUN_CMD} ${BUILD_DIR}/${TARGET_NAME}"
    echo "run:"
    echo $CMD $@
    # eval $CMD
    $CMD $@
}

function place_time_cursor() {
    CURSOR_TAG="$@"
    CURSOR_TIME="$(date)"
}

function dump_time() {
    CURRENT_TIME="$(date)"
    echo ""
    echo "$CURSOR_TAG"
    echo "beginAt: ${CURSOR_TIME}"
    echo "current: ${CURRENT_TIME}"
}

case $1 in
makefile)
    regen_mk
    ;;
compiledb)
    regen_compiledb
    ;;
clangd)
    shift 1
    echo "$BASH_SOURCE clangd $@"
    regen_clangd_config $@
    ;;
clean)
    clean_target
    ;;
rebuild)
    place_time_cursor $0 $@
    rebuild_target
    dump_time
    ;;
build)
    place_time_cursor $0 $@
    build_target
    dump_time
    ;;
run)
    shift 1
    run_target $@
    ;;
*)
    place_time_cursor $0 $@
    regen_mk
    regen_compiledb >/dev/null 2>&1
    regen_clangd_config ".clangd"
    rebuild_target
    dump_time
    echo ""
    run_target $@
    dump_time
    ;;
esac
