#!/bin/bash

export IS_CLANG=true

function escape_win_path() {
	local TMPPATH="$(realpath "$1")"
	if test $? -ne 0; then
		echo "$1"
		return
	fi
	if test "$(uname)" = "Linux"; then
		echo "$TMPPATH"
	else
		which cygpath >/dev/null
		if test $? -ne 0; then
			echo "$TMPPATH"
		else
			cygpath -m "$TMPPATH"
		fi
	fi
}
export -f escape_win_path

# FIXME: clang PATH
TOOLCHAIN_BIN_DIR=${TOOLCHAIN_BIN_DIR="$(escape_win_path "$(dirname "$(which clang)")")"}
if test -d "${TOOLCHAIN_BIN_DIR}"; then
	export PATH="${TOOLCHAIN_BIN_DIR}:$PATH"
fi

# FIXME: cross-compile
TOOLCHAIN_TRIPLE=${TOOLCHAIN_TRIPLE=""}
if test -n "${TOOLCHAIN_TRIPLE}"; then
	export CROSS_COMPILE="${TOOLCHAIN_TRIPLE}-"
	export CLANG_TARGET="--target=${TOOLCHAIN_TRIPLE}"
fi

SYSROOT_DEFAULT=""

# FIXME:
SYSROOT=${SYSROOT="${SYSROOT_DEFAULT}"}
#
if test -z "${SYSROOT}"; then
	SYSROOT="$SYSROOT_DEFAULT"
fi
export SYSROOT

if test "${SYSROOT}" = "${SYSROOT_DEFAULT}"; then
	echo "" >/dev/null
elif test -z "${SYSROOT}"; then
	echo "" >/dev/null
elif test -z "${SYSROOT_DEFAULT}"; then
	echo "" >/dev/null
	SYSROOT_CFLAGS="--sysroot=${SYSROOT}"
elif test "$(realpath "${SYSROOT}")" = "$(realpath "${SYSROOT_DEFAULT}")"; then
	echo "" >/dev/null
else
	SYSROOT_CFLAGS="--sysroot=${SYSROOT}"
fi

# FIXME:
export SYSTEM_NAME=${SYSTEM_NAME="$(uname)"}
# FIXME:
export SYSTEM_PROCESSOR=${SYSTEM_PROCESSOR="$(uname -m)"}
#
EXPORT_TOOLCHAIN=${EXPORT_TOOLCHAIN=true}

# FIXME:
CFLAGS=${CFLAGS=""}
CFLAGS="${CLANG_TARGET} ${CFLAGS} ${SYSROOT_CFLAGS}"
export CFLAGS

CXXFLAGS=${CXXFLAGS=""}
CXXFLAGS="${CLANG_TARGET} ${CXXFLAGS} ${SYSROOT_CFLAGS}"
export CXXFLAGS

CPPFLAGS=${CPPFLAGS=""}
export CPPFLAGS

LDFLAGS=${LDFLAGS=""}
export LDFLAGS

LIBS=${LIBS=""}
export LIBS

CC=${CC=clang}
CXX=${CXX=clang++}
AR=${AR=llvm-ar}
RANLIB=${RANLIB=llvm-ranlib}
ADDR2LINE=${ADDR2LINE=llvm-addr2line}
OBJCPY=${OBJCPY=llvm-objcopy}
STRIP=${STRIP=llvm-strip}

function export_toolchain() {
	export CC
	export CXX
	export AR
	export RANLIB
	export ADDR2LINE
	export OBJCPY
	export STRIP
}

if test "${EXPORT_TOOLCHAIN}" = "true"; then
	export_toolchain
fi

CLANG_RT_DIR="$(escape_win_path "$(clang --print-runtime-dir)")"
# for Linux
export LD_LIBRARY_PATH="${CLANG_RT_DIR}:${LD_LIBRARY_PATH}"
# for Windows
export PATH="${CLANG_RT_DIR}:${PATH}"
