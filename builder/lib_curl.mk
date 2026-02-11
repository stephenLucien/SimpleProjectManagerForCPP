
# add libcurl
$(call pkgconf_add, libcurl)
CPPFLAGS += -DHAVE_LIBCURL
