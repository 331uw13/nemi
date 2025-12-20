TARGET = nemi
CC = gcc
CCFLAGS = \
        -O2 \
        -Wall \
        -Wextra \
        -D_REENTRANT \
        -D_GNU_SOURCE \
        -fwrapv \
        -fno-strict-aliasing \
        -pipe \
        -fstack-protector-strong \
        -I/usr/include/db5.3 \
        -D_LARGEFILE_SOURCE \
        -D_FILE_OFFSET_BITS=64 \
        -I/usr/lib/perl5/5.42/core_perl/CORE \
        -I/usr/include/freetype2 \
        -I/usr/include/libpng16 \
        -I./libs/libvterm-0.3.3/include \
        `perl -MExtUtils::Embed -e ccopts -e ldopts` 

LDFLAGS = \
        -lglfw \
        -lGL \
        -lGLEW \
        -lm \
        -lfreetype \
        -lperl \
        -lvterm_l \
        -Wl,-rpath=/usr/lib/perl5/5.42/core_perl/CORE -L/usr/lib/perl5/5.42/core_perl/CORE \
        -Wl,-rpath=./libs/libvterm-0.3.3 -L./libs/libvterm-0.3.3 


SRC = $(shell find ./src -type f -name *.c)
SRC += xs/xsinit.c
SRC += xs/nemi.c
OBJS = $(SRC:.c=.o)

all: pre-build nemi

pre-build:
	@perl -MExtUtils::Embed -e xsinit -- -o xs/xsinit.c
	@xsubpp xs/nemi.xs > xs/nemi.c

%.o: %.c
	$(CC) $(CCFLAGS) -c $< -o $@

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) -o $@

clean:
	rm -v xs/nemi.c
	rm -v xs/xsinit.c
	rm -v $(OBJS) $(TARGET)

