#!/bin/bash

#
function test_cmd_exist_only() {
	local TMP_CMD="$1"
	which ${TMP_CMD} >/dev/null 2>&1
	if test $? -ne 0; then
		test -n "${TMP_CMD}" && echo "${TMP_CMD} not found!!!"
	fi
}

#
function test_cmd_exist_exit() {
	local TMP_CMD="$1"
	which ${TMP_CMD} >/dev/null 2>&1
	if test $? -ne 0; then
		test -n "${TMP_CMD}" && echo "${TMP_CMD} not found!!!"
		exit 1
	fi
}

test_cmd_exist_exit bash
test_cmd_exist_exit basename
test_cmd_exist_exit dirname
test_cmd_exist_exit realpath
test_cmd_exist_exit awk
test_cmd_exist_exit seq

SCRIPT_DIR=$(dirname $(realpath $BASH_SOURCE))

SRC_DIR=${SRC_DIR=${SCRIPT_DIR}/../src}
BUILD_DIR=${BUILD_DIR=${SCRIPT_DIR}/../build}
TARGET_NAME=${TARGET_NAME=exec}

ABS_SRC_DIR=$(realpath $SRC_DIR)
ABS_BUILD_DIR=$(realpath $BUILD_DIR)
GLOB_RULES_DIR=${GLOB_RULES_DIR=${SCRIPT_DIR}}

test -d "${ABS_SRC_DIR}" || mkdir -p ${ABS_SRC_DIR}
test -d "${ABS_BUILD_DIR}" || mkdir -p ${ABS_BUILD_DIR}
test -d "${GLOB_RULES_DIR}" || mkdir -p ${GLOB_RULES_DIR}

C_SRC_RULE_LIST=${GLOB_RULES_DIR}/.glob_rule_c
CXX_SRC_RULE_LIST=${GLOB_RULES_DIR}/.glob_rule_cxx
IGNORE_SRC_RULE_LIST=${GLOB_RULES_DIR}/.glob_rule_ignore

RELATIVE_PATH=${ABS_BUILD_DIR}/.glob_relative
C_SRC_LIST=${ABS_BUILD_DIR}/.glob_c
CXX_SRC_LIST=${ABS_BUILD_DIR}/.glob_cxx
IGNORE_SRC_LIST=${ABS_BUILD_DIR}/.glob_ignore
SRC_MK=${ABS_BUILD_DIR}/src.mk
ENTRY_MK=${ABS_BUILD_DIR}/makefile
INIT_MK=${GLOB_RULES_DIR}/makefile.init.mk
DEFS_MK=${GLOB_RULES_DIR}/makefile.defs.mk
TARGET_MK=${GLOB_RULES_DIR}/makefile.target.mk

# relative to ${ENTRY_MK}
OBJ_DIR=objs

function find_line() {
	local F="$1"
	local L="$2"
	echo "" | cat "$F" - | while read LINE; do
		if test -z "${L}"; then
			break
		fi
		if test -z "${LINE}"; then
			continue
		fi
		if test "$L" = "$LINE"; then
			echo 1
			return 1
		fi
	done
	echo 0
	return 0
}

function exclude_file() {
	local INC_LIST="$1"
	local EXC_LIST="$2"
	local RES
	echo "" | cat "${INC_LIST}" - | while read LINE; do
		if test -z "${LINE}"; then
			continue
		fi
		RES=$(find_line "$EXC_LIST" "$LINE")
		if test "${RES}" = "0"; then
			echo $LINE
		fi
	done
}

function glob_file() {
	local RULE_LIST=$(realpath "$1")
	local TMP_RULE
	local TMP_FILE
	local GLOB_LIST=$(realpath "$2")
	local REF_DIR=$(dirname ${GLOB_LIST})
	if test -e "${RULE_LIST}"; then
		(
			cd $(dirname ${RULE_LIST})
			echo "" | cat ${RULE_LIST} - | while read LINE; do
				TMP_RULE=$(echo $LINE | awk -F'#' '{print $1}')
				# echo "${TMP_RULE}"
				if test -n "$TMP_RULE"; then
					eval "$TMP_RULE" | while read TMP_FILE; do
						realpath -m --relative-to=${REF_DIR} $TMP_FILE
					done
				fi
			done >${GLOB_LIST}
		)
	fi
}

function glob_cxx_source_files() {
	realpath -m --relative-to=${ABS_BUILD_DIR} ${ABS_SRC_DIR} >${RELATIVE_PATH}
	glob_file "${IGNORE_SRC_RULE_LIST}" "${IGNORE_SRC_LIST}"
	glob_file "${C_SRC_RULE_LIST}" "${C_SRC_LIST}"
	glob_file "${CXX_SRC_RULE_LIST}" "${CXX_SRC_LIST}"
}

function pretty_print_args() {
	local args=($@)
	local idx
	local tmpcmd
	# newline
	echo ' \'
	for idx in $(seq 1 $#); do
		# print an arg at one line
		tmpcmd="echo -n \${args[$(($idx - 1))]}"
		eval $tmpcmd
		echo ' \'
	done
	# newline
	echo ""
}
export -f pretty_print_args

function glob_list_to_mk() {
	local SRC_RELATIVE_PATH="$(cat ${RELATIVE_PATH})"
	local STRIP_PATH
	if test "${SRC_RELATIVE_PATH}" = "" -o "${SRC_RELATIVE_PATH}" = "."; then
		STRIP_PATH=""
	else
		STRIP_PATH="${SRC_RELATIVE_PATH}/"
	fi

	C_SRCS=($(exclude_file "${C_SRC_LIST}" "${IGNORE_SRC_LIST}" | tr '\n' ' '))
	echo "C sources(cnt=${#C_SRCS[@]}):"
	for TMP_ELE_POS in $(seq 1 1 ${#C_SRCS[@]}); do
		TMP_ELE_IDX=$(($TMP_ELE_POS - 1))
		echo "${C_SRCS[$TMP_ELE_IDX]}"
	done
	echo ""

	CXX_SRCS=($(exclude_file "${CXX_SRC_LIST}" "${IGNORE_SRC_LIST}" | tr '\n' ' '))
	echo "CXX sources(cnt=${#CXX_SRCS[@]}):"
	for TMP_ELE_POS in $(seq 1 1 ${#CXX_SRCS[@]}); do
		TMP_ELE_IDX=$(($TMP_ELE_POS - 1))
		echo "${CXX_SRCS[$TMP_ELE_IDX]}"
	done
	echo ""

	local GLOB_THIRDPARTY_LIBS=()
	if test -d "${SRC_DIR}/thirdparty/libs"; then
		local tmpar
		for tmpar in $(ls ${SRC_DIR}/thirdparty/libs/lib*.a); do
			if test -e "$tmpar"; then
				local tmp_ar_filename=$(basename "$tmpar")
				GLOB_THIRDPARTY_LIBS+=(-l:${tmp_ar_filename})
			fi
		done
		local tmpso
		for tmpso in $(ls ${SRC_DIR}/thirdparty/libs/lib*.so); do
			if test -e "$tmpso"; then
				local tmp_so_filename=$(basename "$tmpso")
				local tmp_libname0=${tmp_so_filename:3}
				local tmp_libname1=${tmp_libname0%.so}
				GLOB_THIRDPARTY_LIBS+=(-l${tmp_libname1})
			fi
		done
	fi

	cat >${SRC_MK} <<EOF
# glob by ${BASH_SOURCE}

STRIP_PATH = ${STRIP_PATH}

INCLUDES += -I${SRC_RELATIVE_PATH}
INCLUDES += -I${SRC_RELATIVE_PATH}/thirdparty

LDFLAGS += -I${SRC_RELATIVE_PATH}/thirdparty/libs
LIBS +=$(pretty_print_args ${GLOB_THIRDPARTY_LIBS[@]})

C_SRCS =$(pretty_print_args ${C_SRCS[@]})

CXX_SRCS =$(pretty_print_args ${CXX_SRCS[@]})

C_OBJS = \$(patsubst \${STRIP_PATH}%, ${OBJ_DIR}/%.c.o, \${C_SRCS})
CXX_OBJS = \$(patsubst \${STRIP_PATH}%, ${OBJ_DIR}/%.cxx.o, \${CXX_SRCS})
OBJS = \${C_OBJS} \${CXX_OBJS}

C_DEPS = \$(patsubst %.c.o, %.d, \${C_OBJS})
CXX_DEPS = \$(patsubst %.cxx.o, %.d, \${CXX_OBJS})
DEPS = \${C_DEPS} \${CXX_DEPS}
ifneq (\$(strip \${DEPS}),)
-include \${DEPS}
endif

${OBJ_DIR}/%.c.o: \${STRIP_PATH}%
	@echo compiling "\$<"
	@\$(CC) \$(CFLAGS) \$(INCLUDES) \$(CPPFLAGS) -c -MMD -MP -MF"\$(@:%.c.o=%.d)" -MT"\$(@)" -o "\$@" "\$<"

${OBJ_DIR}/%.cxx.o: \${STRIP_PATH}%
	@echo compiling "\$<"
	@\$(CXX) \$(CXXFLAGS) \$(INCLUDES) \$(CPPFLAGS) -c -MMD -MP -MF"\$(@:%.cxx.o=%.d)" -MT"\$(@)" -o "\$@" "\$<"

EOF

	cat >${ENTRY_MK} <<EOF

-include $(realpath -m --relative-to=$(dirname ${ENTRY_MK}) ${INIT_MK})

# COM_COMPILE_FLAGS += -Os
# COM_COMPILE_FLAGS += -fPIC
# COM_COMPILE_FLAGS += -save-temps=obj
# COM_COMPILE_FLAGS += -fdata-sections -ffunction-sections
# COM_COMPILE_FLAGS += -fno-omit-frame-pointer
# COM_COMPILE_FLAGS += -fstack-protector
# COM_COMPILE_FLAGS += -fno-caller-saves 
# COM_COMPILE_FLAGS += -fmessage-length=0
# COM_COMPILE_FLAGS += -Wno-unused-result -Wno-unused-but-set-variable -Wno-unused-but-set-parameter -Wno-unused-variable -Wno-unused-parameter -Wno-unused-label -Wno-unused-function

# CFLAGS += \${COM_COMPILE_FLAGS}
# CXXFLAGS += \${COM_COMPILE_FLAGS} -fexceptions

# LDFLAGS += -Wl,--gc-sections

TARGET = ${TARGET_NAME}

.PHONY: \${TARGET} \${TARGET}.gnu_debuglink

WHOLE_OBJS_ARCHIVE = objs.whole.a

all: dump_compile_info \${TARGET} \${TARGET}.gnu_debuglink

-include $(realpath -m --relative-to=$(dirname ${ENTRY_MK}) ${DEFS_MK})

# \$(call rm_duplicate,CFLAGS)
# \$(call rm_duplicate,CXXFLAGS)
# \$(call rm_duplicate,INCLUDES)
# \$(call rm_duplicate,CPPFLAGS)
# \$(call rm_duplicate,LDFLAGS)
# \$(call rm_duplicate,LIBS)

include $(basename ${SRC_MK})

\${TARGET}.gnu_debuglink: \${TARGET}.strip \${TARGET}.symbols
	@\${OBJCPY} --add-gnu-debuglink=\${TARGET}.symbols \${TARGET}.strip  

\${TARGET}.strip: \${TARGET}
	@\${OBJCPY} --strip-debug \$< \$@

\${TARGET}.symbols: \${TARGET}
	@\${OBJCPY} --only-keep-debug \$< \$@

\${TARGET}: \${WHOLE_OBJS_ARCHIVE}
ifeq (\$(IS_WINDOWS), true)
	@\$(CXX) \$(CXXFLAGS) \$(LDFLAGS) -Wl,/WHOLEARCHIVE:\$^ \$(LIBS) -o \$@
else
	@\$(CXX) \$(CXXFLAGS) \$(LDFLAGS) -Wl,--whole-archive \$^ -Wl,--no-whole-archive \$(LIBS) -o \$@
endif

\${WHOLE_OBJS_ARCHIVE}: \${OBJS}
	@rm -f \$@
	@\$(AR) rcsP \$@ $^

.PHONY: c_compiler cxx_compiler cppflags
c_compiler:
	@echo \$(CC) >\$@
cxx_compiler:
	@echo \$(CXX) >\$@
cppflags:
	@echo \$(INCLUDES) \$(CPPFLAGS) >\$@

dump_compile_info: c_compiler cxx_compiler cppflags

clean::
	@rm -rf \${OBJS} \${DEPS}

-include $(realpath -m --relative-to=$(dirname ${ENTRY_MK}) ${TARGET_MK})

EOF

	(
		cd ${ABS_BUILD_DIR}
		rm -rf ${OBJ_DIR}/
		cat ${C_SRC_LIST} ${CXX_SRC_LIST} | while read SF; do
			OF=${OBJ_DIR}/${SF#${STRIP_PATH}}.o
			# echo $OF
			mkdir -p $(dirname ${OF})
		done
	)

}

glob_cxx_source_files
glob_list_to_mk
