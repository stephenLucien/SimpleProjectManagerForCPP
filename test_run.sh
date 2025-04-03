#!/bin/sh

TARGET_HOST=${TARGET_HOST=host_asan}

if test "1" = "0"; then
    ./compile.sh
else
    ./compile.sh build
fi

./compile.sh run --test "$*"
