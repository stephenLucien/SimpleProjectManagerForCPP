
# address sanitizer
CFLAGS += -fsanitize=address
CXXFLAGS += -fsanitize=address
LIBS += -lasan

# 
LIBS += -lstdc++
LIBS += -lpthread

# add openssl
$(call pkgconf_add, openssl)

# add libcurl
$(call pkgconf_add, libcurl)

