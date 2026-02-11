ifeq ($(IS_WINDOWS), true)
# $(shell cp "${rpathd}/../src/thirdparty/libs/*.dll" .)
else
LIBS += -ldl
LIBS += -lpthread
endif
