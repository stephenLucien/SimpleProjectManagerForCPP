#!/bin/bash
CUR_DIR="$(dirname "$(realpath "${BASH_SOURCE[0]}")")"

export IS_WINDOWS=true

CC=${CC=clang.exe}
CXX=${CXX=clang.exe}
AR=${AR=llvm-ar.exe}
RANLIB=${RANLIB=llvm-ranlib.exe}
ADDR2LINE=${ADDR2LINE=llvm-addr2line.exe}
OBJCPY=${OBJCPY=llvm-objcopy.exe}
STRIP=${STRIP=llvm-strip.exe}

CFLAGS=
# CFLAGS="${CFLAGS} -fPIC"
# CFLAGS="${CFLAGS} -fdata-sections -ffunction-sections"
CFLAGS="${CFLAGS} -fno-omit-frame-pointer"
CFLAGS="${CFLAGS} -save-temps=obj"
CFLAGS="${CFLAGS} -Wno-unused-result -Wno-unused-but-set-variable -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-parameter -Wno-unused-label -Wno-unused-function"
CFLAGS="${CFLAGS} -O2"

CXXFLAGS="${CFLAGS} -fexceptions"

INCLUDES=""

CPPFLAGS=""

# LDFLAGS="-Wl,--gc-sections"

LIBS=""

source "${CUR_DIR}/env_template_for_clang.sh"
