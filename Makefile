


CC = gcc

COMPILE_FLAGS = -ggdb \
				-Wall \
				-Wextra \
				-D_LARGEFILE_SOURCE \
				-D_LARGEFILE64_SOURCE \
				-D_FILE_OFFSET_BITS=64 \
				`perl -MExtUtils::Embed -e ccopts -e ldopts`

TARGET_NAME = nemi

# TODO: Create some configuration script?
#       Everyone may not have this perl version installed.
#
LIB_VTERM = ./libs/libvterm-0.3.3
LIB_PERL = /usr/lib/perl5/5.42/core_perl/CORE



SRC  = $(shell find ./src -type f -name "*.c")
SRC += xs/xsinit.c
SRC += xs/nemi.c


OBJS = $(SRC:.c=.o)
LIBS = -lglfw -lGL -lGLEW -lm -lfreetype -lperl -lvterm_l

HEADER_INCLUDE_PATHS = \
				-I$(LIB_PERL) \
			    -I$(LIB_VTERM)/include \
			    -I/usr/include/freetype2 \
			    -I/usr/include/libpng16 

LIB_INCLUDE_PATHS = -Wl,-rpath=$(LIB_PERL) -L$(LIB_PERL) \
					-Wl,-rpath=$(LIB_VTERM) -L$(LIB_VTERM)

all: pre-build $(TARGET_NAME)

pre-build:
	@perl -MExtUtils::Embed -e xsinit -- -o xs/xsinit.c
	@xsubpp xs/nemi.xs > xs/nemi.c

%.o: %.c
	@$(CC) $(HEADER_INCLUDE_PATHS) $(COMPILE_FLAGS) \
		-c $< -o $@ && (echo -e "\033[32m[Compiled]\033[0m $<") || (echo -e "\033[31m[Failed]\033[0m $<"; exit 1) 

$(TARGET_NAME): $(OBJS)
	@echo -e "\033[90mLinking...\033[0m"
	@$(CC) $(OBJS) $(LIB_INCLUDE_PATHS) $(LIBS) \
		-o $@ && (echo -e "\033[36mDone.\033[0m"; ls -lh $(TARGET_NAME))

clean:
	rm -v xs/nemi.c
	rm -v xs/xsinit.c
	@rm -v $(OBJS) $(TARGET_NAME)

.PHONY: all clean

