# add opencv4
ifeq ($(IS_WINDOWS), true)
LIBS += -lopencv_world4120
LIBS += -lopencv_videoio_ffmpeg4120_64
else
$(call pkgconf_add, opencv4)
endif
