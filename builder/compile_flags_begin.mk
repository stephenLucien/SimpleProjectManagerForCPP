# standard
CFLAGS += -std=gnu11
CXXFLAGS += -std=gnu++14

# optimize level
CFLAGS += -O0 -g
CXXFLAGS += -O0 -g

# address sanitizer
CFLAGS += -fsanitize=address
CXXFLAGS += -fsanitize=address

LIBS += -lasan

# LDFLAGS += -Wl,--as-needed
# LDFLAGS += -Wl,--allow-shlib-undefined

$(call append_system_ld_library_path)
