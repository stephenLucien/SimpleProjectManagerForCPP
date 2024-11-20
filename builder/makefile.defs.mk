
# address sanitizer
CFLAGS += -std=gnu11
CFLAGS += -fsanitize=address

CXXFLAGS += -std=gnu++14
CXXFLAGS += -fsanitize=address

# LDFLAGS += -Wl,--as-needed
# LDFLAGS += -Wl,--allow-shlib-undefined

LIBS += -lasan

# pkgconf --list-all | awk '{print $1}' | grep opencv
# $(call pkgconf_add, opencv4)
# $(call pkgconf_add, protobuf)

# add libcurl
$(call pkgconf_add, libcurl)

# add openssl
$(call pkgconf_add, openssl)

# libqrencode
# $(call pkgconf_add, libqrencode)

# 
LIBS += -lstdc++
LIBS += -lpthread
