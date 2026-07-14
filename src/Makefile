CFLAGS += -Wall -Werror -pedantic
LDFLAGS := -lserialport -lubus -lubox -lblobmsg_json -lcjson

SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
EXEC = ubus-esp-controller

all: build

build: $(OBJS)
	$(CC) -o $(EXEC) $^ $(LDFLAGS)

clean:
	rm -f *.o
	rm -f $(EXEC)

.PHONY: all build clean