FLAGS = -ggdb -Wall -Wextra
CC = gcc

TARGET_NAME = nemi

SRC  = $(shell find ./src -type f -name *.c)
OBJS = $(SRC:.c=.o)
LIBS = -lglfw -lGL -lGLEW -lm -lfreetype -lutil


all: $(TARGET_NAME)

%.o: %.c
	@$(CC) $(FLAGS) -I/usr/include/freetype2 -I/usr/include/libpng16 \
		-c $< -o $@ && (echo -e "\033[32m[Compiled]\033[0m $<") || (echo -e "\033[31m[Failed]\033[0m $<"; exit 1) 

$(TARGET_NAME): $(OBJS)
	@echo -e "\033[90mLinking...\033[0m"
	@$(CC) $(OBJS) $(LIBS) -o $@ && (echo -e "\033[36mDone.\033[0m"; ls -lh $(TARGET_NAME))

clean:
	@rm -v $(OBJS) $(TARGET_NAME)

.PHONY: all clean

