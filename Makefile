SRC_DIR = src

all: src

src:
	$(MAKE) -C $(SRC_DIR)

clean:
	$(MAKE) -C $(SRC_DIR) clean

.PHONY: all src clean