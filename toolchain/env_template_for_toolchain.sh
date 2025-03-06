#!/bin/bash

# FIXME: gcc PATH
TOOLCHAIN_BIN_DIR=${TOOLCHAIN_BIN_DIR=$(dirname $(which gcc))}
if test -d "${TOOLCHAIN_BIN_DIR}"; then
    export PATH="${TOOLCHAIN_BIN_DIR}:$PATH"
fi

# FIXME: cross-compile
TOOLCHAIN_TRIPLE=${TOOLCHAIN_TRIPLE=""}
if test -n "${TOOLCHAIN_TRIPLE}"; then
    export CROSS_COMPILE="${TOOLCHAIN_TRIPLE}-"
fi
SYSROOT_DEFAULT=$(${CROSS_COMPILE}gcc --print-sysroot | sed -e 's/\\/\//g')

# FIXME:
SYSROOT=${SYSROOT=${SYSROOT_DEFAULT}}
#
if test -z "${SYSROOT}"; then
    SYSROOT=$SYSROOT_DEFAULT
fi
export SYSROOT

if test "${SYSROOT}" = "${SYSROOT_DEFAULT}"; then
    echo "" >/dev/null
elif test -z "${SYSROOT}"; then
    echo "" >/dev/null
elif test -z "${SYSROOT_DEFAULT}"; then
    echo "" >/dev/null
    SYSROOT_CFLAGS="--sysroot=${SYSROOT}"
elif test "$(realpath ${SYSROOT})" = "$(realpath ${SYSROOT_DEFAULT})"; then
    echo "" >/dev/null
else
    SYSROOT_CFLAGS="--sysroot=${SYSROOT}"
fi

# FIXME:
export SYSTEM_NAME=${SYSTEM_NAME=$(uname)}
# FIXME:
export SYSTEM_PROCESSOR=${SYSTEM_PROCESSOR=$(uname -m)}
#
EXPORT_TOOLCHAIN=${EXPORT_TOOLCHAIN=true}

# FIXME:
CFLAGS=${CFLAGS=""}
CFLAGS="${CFLAGS} ${SYSROOT_CFLAGS}"
export CFLAGS

CXXFLAGS=${CXXFLAGS=""}
CXXFLAGS="${CXXFLAGS} ${SYSROOT_CFLAGS}"
export CXXFLAGS

CPPFLAGS=${CPPFLAGS=""}
export CPPFLAGS

LDFLAGS=${LDFLAGS=""}
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
