# $(call get_relative_path, rpathf)
# $(call get_absolute_path, apathf)
$(call get_relative_dir, rpathd)
# $(call get_absolute_dir, apathd)

-include ${rpathd}/compile_flags_begin.mk


# -include ${rpathd}/lib_nanogui.mk
# -include ${rpathd}/lib_opencv4.mk
# -include ${rpathd}/lib_protobuf.mk
# -include ${rpathd}/lib_glew.mk
# -include ${rpathd}/lib_qrencode.mk
# -include ${rpathd}/lib_curl.mk
# -include ${rpathd}/lib_openssl.mk

# -include ${rpathd}/lib_alsa.mk

-include ${rpathd}/compile_flags_end.mk
