FILES = preztool.c
LIBRARIES = -lraylib

ifeq ($(OS),Windows_NT)
    LIBRARIES += -lgdi32 -lwinmm -Ivendor/raylib/include -Lvendor/raylib/lib
    FILES += preztool_win32.c
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Linux)
        LIBRARIES += -lX11
	FILES += preztool_x11.c
    endif
    ifeq ($(UNAME_S),Darwin)
	ERROR := $(error no support for darwin machines)
    endif
endif

all:
	gcc $(FILES)\
		-march=native -O3\
		$(LIBRARIES)\
		-o preztool
