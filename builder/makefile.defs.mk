# $(call get_relative_path, rpathf)
# $(call get_absolute_path, apathf)
$(call get_relative_dir, rpathd)
# $(call get_absolute_dir, apathd)

-include ${rpathd}/compile_flags_begin.mk

# -include ${rpathd}/lib_curl.mk
# -include ${rpathd}/lib_openssl.mk
# -include ${rpathd}/lib_zlib.mk

-include ${rpathd}/compile_flags_end.mk
