# 
define get_relative_path
$(eval $(1) := $(lastword $(MAKEFILE_LIST)))
endef
#
define get_absolute_path
$(eval $(1) := $(abspath $(lastword $(MAKEFILE_LIST))))
endef
#
define get_relative_dir
$(eval $(1) := $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST)))))
endef
#
define get_absolute_dir
$(eval $(1) := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST))))))
endef

# append LD_LIBRARY_PATH to LDFALGS
define append_system_ld_library_path
$(eval LDFLAGS += $(patsubst %, -L%, $(shell echo ${LD_LIBRARY_PATH} | tr ':' '\n' | awk '!seen[$$0]++' | tr '\n' ' ')))
endef

# show all installed pkg on host: pkgconf --list-all
# Add libcurl:
# $(call pkgconf_add libcurl)
define pkgconf_add_alt
$(eval INCLUDES += $(shell pkg-config --cflags-only-I $(1)))
$(eval CPPFLAGS += $(shell pkg-config --cflags-only-other $(1)))
$(eval LDFLAGS += $(shell pkg-config --libs-only-L $(1)))
$(eval LIBS += $(shell pkg-config --libs-only-l $(1)))
endef

define pkgconf_add
$(eval INCLUDES += $(shell pkg-config --cflags-only-I $(1)))
$(eval LDFLAGS += $(shell pkg-config --libs-only-L $(1)))
$(eval LIBS += $(shell pkg-config --libs-only-l $(1)))
endef

define rm_duplicate
$(eval $(1) := $(shell echo ${$(1)} | tr ' ' '\n' | awk '!seen[$$0]++'))
endef

define rm_duplicate_from
$(eval $(1) := $(shell echo ${$(2)} | tr ' ' '\n' | awk '!seen[$$0]++'))
endef
