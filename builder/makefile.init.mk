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
