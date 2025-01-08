RAYLIB ?= ./vendor/raylib/src/
FILES = preztool.c
LIBRARIES = -lraylib

ifeq ($(OS),Windows_NT)
	LIBRARIES += -lgdi32 -lwinmm
	FILES += preztool_win32.c
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
	LIBRARIES += -lm -lX11
	FILES += preztool_x11.c
    endif
    ifeq ($(UNAME_S),Darwin)
	ERROR := $(error no support for darwin machines)
    endif
endif

ifneq ($(STATIC_LINK_RAYLIB),)
	LIBRARIES += -I $(RAYLIB) -L $(RAYLIB)
endif

all:
ifneq ($(PLATFORM),)
	cd vendor/raylib/src &&\
		make PLATFORM=$(PLATFORM) RAYLIB_BUILD_MODE=$(BUILD_MODE)
endif
	gcc $(FILES)\
		-march=native -O3 -Wall -Wextra\
		$(LIBRARIES)\
		-o preztool

static:
	make PLATFORM=PLATFORM_DESKTOP STATIC_LINK_RAYLIB=t
