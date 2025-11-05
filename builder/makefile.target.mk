ifeq ($(IS_WINDOWS), true)
$(shell cp "${rpathd}/../src/thirdparty/libs/*.dll" .)
endif
