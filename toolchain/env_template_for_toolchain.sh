#!/bin/bash
CUR_DIR=$(dirname $(realpath ${BASH_SOURCE}))

# FIXME: toolchain directory
TOOLCHAIN_DIR=${TOOLCHAIN_DIR=${CUR_DIR}}

# FIXME: gcc PATH
TOOLCHAIN_BIN_DIR=${TOOLCHAIN_BIN_DIR=${TOOLCHAIN_DIR}/bin}
export PATH="${TOOLCHAIN_BIN_DIR}:$PATH"

# FIXME: cross-compile
TOOLCHAIN_TRIPLE=${TOOLCHAIN_TRIPLE=arm-linux-gnueabihf}
if test -n "${TOOLCHAIN_TRIPLE}"; then
    export CROSS_COMPILE="${TOOLCHAIN_TRIPLE}-"
fi

# FIXME:
export SYSROOT=${SYSROOT=$(${CROSS_COMPILE}gcc --print-sysroot)}
# FIXME:
export SYSTEM_NAME=${SYSTEM_NAME=Linux}
# FIXME:
export SYSTEM_PROCESSOR=${SYSTEM_PROCESSOR=arm}
#
EXPORT_TOOLCHAIN=${EXPORT_TOOLCHAIN=true}

# FIXME:
CFLAGS=${CFLAGS="-mfloat-abi=hard -mfpu=vfp"}
# CFLAGS="${CFLAGS} -fPIC"
# CFLAGS="${CFLAGS} -fdata-sections -ffunction-sections"
# CFLAGS="${CFLAGS} -fno-omit-frame-pointer"
# CFLAGS="${CFLAGS} -Os"
# CFLAGS="${CFLAGS} -pipe"
# CFLAGS="${CFLAGS} -save-temps=obj"
# CFLAGS="${CFLAGS} -Wno-unused-result -Wno-unused-but-set-variable -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-parameter -Wno-unused-label -Wno-unused-function"
export CFLAGS

CXXFLAGS=${CXXFLAGS="${CFLAGS} -fexceptions"}
export CXXFLAGS

CPPFLAGS=${CPPFLAGS=""}
export CPPFLAGS

LDFLAGS=${LDFLAGS="-Wl,--gc-sections"}
export LDFLAGS

LIBS=${LIBS=""}
export LIBS

CC=${CC=${CROSS_COMPILE}gcc}
CXX=${CXX=${CROSS_COMPILE}g++}
AR=${AR=${CROSS_COMPILE}ar}
RANLIB=${RANLIB=${CROSS_COMPILE}ranlib}
ADDR2LINE=${ADDR2LINE=${CROSS_COMPILE}addr2line}
STRIP=${STRIP=${CROSS_COMPILE}strip}

function export_toolchain() {
    export CC
    export CXX
    export AR
    export RANLIB
    export ADDR2LINE
    export STRIP
}

if test "${EXPORT_TOOLCHAIN}" = "true"; then
    export_toolchain
fi
