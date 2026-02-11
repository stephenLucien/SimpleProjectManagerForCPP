# 
ifeq ($(IS_WINDOWS), true)
LIBS += 
else
LIBS += -lstdc++
# LIBS += -ldl
# LIBS += -lpthread
endif
