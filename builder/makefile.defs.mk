# $(call get_relative_path, rpathf)
# $(call get_absolute_path, apathf)
$(call get_relative_dir, rpathd)
# $(call get_absolute_dir, apathd)

ifeq ($(IS_CLANG), true)
-include ${rpathd}/clang_flags_begin.mk
else
-include ${rpathd}/gcc_flags_begin.mk
endif

# -include ${rpathd}/lib_curl.mk
# -include ${rpathd}/lib_openssl.mk
# -include ${rpathd}/lib_zlib.mk
# -include ${rpathd}/lib_opencv4.mk
# -include ${rpathd}/lib_cjson.mk

# -include ${rpathd}/lib_eigen3.mk

ifeq ($(IS_CLANG), true)
-include ${rpathd}/clang_flags_end.mk
else
-include ${rpathd}/gcc_flags_end.mk
endif

