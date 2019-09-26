CXXFLAGS = -I./include -O0 -Wall -ggdb
SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)

main: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

.PHONY: clean
clean:
	rm -f $(OBJ) main
