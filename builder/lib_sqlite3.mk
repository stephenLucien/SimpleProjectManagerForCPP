
# add sqlite3
$(call pkgconf_add, sqlite3)
CPPFLAGS += -DHAVE_SQLITE3
