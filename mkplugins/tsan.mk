COMMON_FLAGS += --sanitize=thread -g3 -fPIE
LIBS += tsan
LDFLAGS += --sanitize=thread -pie -g3
