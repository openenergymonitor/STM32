#BINPATH = /usr/bin
BINPATH = /usr/local/gcc-arm-none-eabi-8-2018-q4-major/bin
CFLAGS += -std=gnu99
LDFLAGS += -u _printf_float
LDFLAGS += -Xlinker --no-wchar-size-warning