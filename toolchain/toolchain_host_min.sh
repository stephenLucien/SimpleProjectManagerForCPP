#!/bin/bash
CUR_DIR=$(dirname $(realpath ${BASH_SOURCE}))

CFLAGS=""
CXXFLAGS="${CFLAGS}"
INCLUDES=""
CPPFLAGS=""
LDFLAGS=""
LIBS=""

source ${CUR_DIR}/env_template_for_toolchain.sh
