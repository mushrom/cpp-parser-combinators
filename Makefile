#CXXFLAGS = -I./include -O0 -pg -Wall -ggdb -std=c++17
CXXFLAGS = -I./include -O3 -Wall -std=c++17
SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)

main: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

.PHONY: clean
clean:
	rm -f $(OBJ) main
