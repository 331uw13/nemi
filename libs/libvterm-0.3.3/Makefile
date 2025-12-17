FLAGS = -ggdb -Wall -Wextra 
CC = gcc


TARGET_NAME = libvterm_l.a

SRC  = $(shell find ./src -type f -name "*.c")
OBJS = $(SRC:.c=.o)

all: $(TARGET_NAME)

%.o: %.c
	@$(CC) $(FLAGS) \
		-c $< -o $@

$(TARGET_NAME): $(OBJS)
	@echo -e "\033[90mLinking...\033[0m"
	@ar rcs $@ $^

clean:
	@rm -v $(OBJS) $(TARGET_NAME)

.PHONY: all clean

