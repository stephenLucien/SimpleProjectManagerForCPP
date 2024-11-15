#!/bin/bash
SCRIPT_DIR=$(dirname $(realpath ${BASH_SOURCE}))

export SRC_DIR=src
export BUILD_DIR=build
export TARGET_NAME=exec

# export CC=clang
# export CXX=clang++
export CC=gcc
export CXX=g++

rm -rf ${BUILD_DIR}
${SCRIPT_DIR}/builder/glob_src.sh
test $? -ne 0 && exit 1

make -C ${BUILD_DIR} clean >/dev/null
make -C ${BUILD_DIR} -n >${BUILD_DIR}/build.log
#
COMPILE_DB_CMD=$(which compiledb)
if test -n "${COMPILE_DB_CMD}"; then
    echo ""
    cd ${BUILD_DIR}
    ${COMPILE_DB_CMD} -v -S -p build.log
    cd -
    cp ${BUILD_DIR}/compile_commands.json .
    echo ""
fi

make -C ${BUILD_DIR} -j1
# make -C ${BUILD_DIR} -j$(nproc)
test $? -ne 0 && exit 1

echo ""
CMD="${BUILD_DIR}/${TARGET_NAME}"
echo "run: $CMD"
eval $CMD
