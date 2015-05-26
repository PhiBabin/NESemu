# File : Makefile
# Author : Émile Robitaille @ L3Robot
# Description : Makefile used to compile NESemu.

EXEC=NESemu

SRC= $(wildcard *.cpp)
OBJ= $(SRC:.cpp=.o)

all: $(EXEC)

NESemu: $(OBJ)
	@$(CXX) -o $@ $^ $(CFLAGS) $(LDFLAGS) $(LIBS) $(SDL)

%.o: %.cpp
	@$(CXX) -o $@ -c $< $(CFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -rf *.o
	rm -rf $(EXEC)
