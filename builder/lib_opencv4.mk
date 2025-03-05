# pkgconf --list-all | awk '{print $1}' | grep opencv
$(call pkgconf_add, opencv4)
