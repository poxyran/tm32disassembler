# tm32disassembler
Source code for the TM32 disassembler created by asbokid

## Cross-compile using MinGW32

Tested on Raspberry Pi:

```
pi@raspberrypi ~/tm32disassembler_1.24b $ uname -a
Linux raspberrypi 3.18.7+ #755 PREEMPT Thu Feb 12 17:14:31 GMT 2015 armv6l GNU/Linux
```

1. sudo apt-get install gcc-mingw32
2. git clone https://github.com/poxyran/tm32disassembler.git
3. cd tm32disassembler
4. make

```
pi@raspberrypi ~/tm32disassembler_1.24b $ make
i586-mingw32msvc-gcc -c -o tm32dis.o tm32dis.c -I. -I./windows -std=c99
i586-mingw32msvc-gcc -c -o tm32main.o tm32main.c -I. -I./windows -std=c99
i586-mingw32msvc-gcc -c -o tm32decode.o tm32decode.c -I. -I./windows -std=c99
i586-mingw32msvc-gcc -c -o tm32funcs.o tm32funcs.c -I. -I./windows -std=c99
i586-mingw32msvc-gcc -c -o tm32memimg.o tm32memimg.c -I. -I./windows -std=c99
i586-mingw32msvc-gcc -c -o tm32unpack.o tm32unpack.c -I. -I./windows -std=c99
i586-mingw32msvc-gcc -o tm32dis tm32dis.o tm32main.o tm32decode.o tm32funcs.o tm32memimg.o tm32unpack.o -I. -I./windows -std=c99
```

