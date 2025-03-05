#!/bin/bash
CUR_DIR=$(dirname $(realpath ${BASH_SOURCE}))

TOOLCHAIN_DIR=
TOOLCHAIN_BIN_DIR=$(dirname $(which gcc))

TOOLCHAIN_TRIPLE=

SYSTEM_NAME=$(uname)

SYSTEM_PROCESSOR=$(uname -m)

CFLAGS=
CFLAGS="${CFLAGS} -fPIC"
CFLAGS="${CFLAGS} -fdata-sections -ffunction-sections"
CFLAGS="${CFLAGS} -fno-omit-frame-pointer"
CFLAGS="${CFLAGS} -save-temps=obj"
CFLAGS="${CFLAGS} -Wno-unused-result -Wno-unused-but-set-variable -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-parameter -Wno-unused-label -Wno-unused-function"
CFLAGS="${CFLAGS} -O2"

CXXFLAGS=${CXXFLAGS="${CFLAGS} -fexceptions"}

LDFLAGS="-Wl,--gc-sections"

LIBS=""

EXPORT_TOOLCHAIN=true
source ${CUR_DIR}/env_template_for_toolchain.sh
