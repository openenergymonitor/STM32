#BINPATH = /usr/bin
#BINPATH = /usr/local/gcc-arm-none-eabi-8-2018-q4-major/bin
BINPATH = /usr/local/gcc-arm-none-eabi-8-2019-q3-update/bin
CFLAGS += -std=gnu99
LDFLAGS += -Xlinker --no-wchar-size-warning -u _printf_float