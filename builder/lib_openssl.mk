# add openssl
$(call pkgconf_add, openssl)
CPPFLAGS += -DHAVE_OPENSSL
