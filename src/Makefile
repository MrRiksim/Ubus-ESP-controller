CFLAGS = -Wall -Werror -pedantic
LDFLAGS = -lserialport -lubus -lubox -lblobmsg_json

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
EXEC = server

all: build

build: $(OBJS)
	$(CC) -o ../$(EXEC) $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o
	rm -f ../$(EXEC)

.PHONY: all build clean