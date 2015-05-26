# File : Makefile
# Author : Émile Robitaille @ L3Robot
# Description : Makefile used to compile NESemu.

export LIBS=$(shell sdl-config --cflags --libs)
export SDL=$(shell sdl-config --cflags)

export CFLAGS=-gdwarf-2 --std=c++11 -I$(shell pwd)
export LDFLAGS=

SRC_DIR=src
EXEC=$(SRC_DIR)/NESemu

all: $(EXEC)

$(EXEC):
	@(cd $(SRC_DIR) && $(MAKE))

.PHONY: clean $(EXEC)

clean:
	@(cd $(SRC_DIR) && $(MAKE) $@)
