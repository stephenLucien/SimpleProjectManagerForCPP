#!/bin/bash
CUR_DIR=$(dirname $(realpath ${BASH_SOURCE}))

# FIXME: important for setup PATH environment
TOOLCHAIN_BIN_DIR=$(dirname $(which arm-linux-gnueabihf-gcc))

# FIXME: important for setup env: CROSS_COMPILE CC CXX AR
TOOLCHAIN_TRIPLE=arm-linux-gnueabihf

# FIXME: setup if sysroot is differ from result of: ${TOOLCHAIN_TRIPLE}-gcc --print-sysroot
SYSROOT=

#
SYSTEM_NAME=Linux

# FIXME: important for qemu, need to setup properly for running cross-compiled program at localhost
SYSTEM_PROCESSOR=arm

# FIXME: important !!!
CFLAGS="-mfloat-abi=hard -mfpu=vfp"


CXXFLAGS="${CFLAGS}"

INCLUDES=""

CPPFLAGS=""

LDFLAGS=""

LIBS=""

# EXPORT_TOOLCHAIN=false
source ${CUR_DIR}/env_template_for_toolchain.sh

