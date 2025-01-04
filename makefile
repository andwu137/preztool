all:
	gcc preztool.c preztool_x11.c\
		-march=native -O3 -lraylib -lX11\
		-o preztool
