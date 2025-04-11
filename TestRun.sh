#!/bin/bash

TARGET_HOST=${TARGET_HOST=host_asan}

if test "1" = "0"; then
    source ./compile.sh
else
    source ./compile.sh build
fi

TESTS=()

# TODO: add tests here
TESTS+=(test_printf_float)

TEST_STR=""
for TMP_TEST_NAME in ${TESTS[@]}; do
    if test -z "${TEST_STR}"; then
        TEST_STR="test_${TMP_TEST_NAME}"
    else
        TEST_STR="${TEST_STR},test_${TMP_TEST_NAME}"
    fi
done
echo ""
CMD="${QEMU_RUN_CMD} ${BUILD_DIR}/${TARGET_NAME}"
echo "run:"
echo $CMD
$CMD --test "${TEST_STR}" $@
