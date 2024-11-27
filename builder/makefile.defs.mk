
# standard
CFLAGS += -std=gnu11
CXXFLAGS += -std=gnu++14

# optimize level
CFLAGS += -O0 -g
CXXFLAGS += -O0 -g


# address sanitizer
CFLAGS += -fsanitize=address
CXXFLAGS += -fsanitize=address

# LDFLAGS += -Wl,--as-needed
# LDFLAGS += -Wl,--allow-shlib-undefined

LIBS += -lasan

# pkgconf --list-all | awk '{print $1}' | grep opencv
# $(call pkgconf_add, opencv4)
# $(call pkgconf_add, protobuf)
# $(call pkgconf_add, glew)

# add libcurl
# $(call pkgconf_add, libcurl)

# add openssl
# $(call pkgconf_add, openssl)

# libqrencode
# $(call pkgconf_add, libqrencode)

# 
LIBS += -lstdc++
LIBS += -lpthread
