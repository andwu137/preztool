all:
	gcc preztool.c preztool_x11.c\
		-march=native -O3 -lm -lraylib -lX11\
		-o preztool
