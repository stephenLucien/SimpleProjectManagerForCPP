#!/bin/bash
CUR_DIR=$(dirname $(realpath ${BASH_SOURCE}))

export IS_WINDOWS=true

CFLAGS=
# CFLAGS="${CFLAGS} -fPIC"
CFLAGS="${CFLAGS} -fdata-sections -ffunction-sections"
CFLAGS="${CFLAGS} -fno-omit-frame-pointer"
CFLAGS="${CFLAGS} -save-temps=obj"
CFLAGS="${CFLAGS} -Wno-unused-result -Wno-unused-but-set-variable -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-parameter -Wno-unused-label -Wno-unused-function"
CFLAGS="${CFLAGS} -O2"
CFLAGS="${CFLAGS} -static"

CXXFLAGS="${CFLAGS} -fexceptions"

INCLUDES=""

CPPFLAGS=""

LDFLAGS="-Wl,--gc-sections"

LIBS=""

source ${CUR_DIR}/env_template_for_toolchain.sh
