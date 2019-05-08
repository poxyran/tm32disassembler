CC=gcc
#CC=i586-mingw32msvc-gcc
CFLAGS=-I. -I./windows -std=c99
DEPS = tm32dis.h tm32disinstrs.h
OBJ = tm32dis.o tm32main.o tm32decode.o tm32funcs.o tm32memimg.o tm32unpack.o

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

tm32dis: $(OBJ)
	gcc -o $@ $^ $(CFLAGS)

.PHONY: clean

clean:
	rm -f *.o *~
