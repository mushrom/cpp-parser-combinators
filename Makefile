#CXXFLAGS = -I./include -O0 -Wall -ggdb -std=c++20
#CXXFLAGS = -I./include -O0 -pg -Wall -ggdb -std=c++20
CXXFLAGS = -I./include -O3 -Wall -std=c++20
SRC = $(wildcard src/*.cpp)
OBJ = $(SRC:.cpp=.o)

main: $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $(OBJ)

.PHONY: clean
clean:
	rm -f $(OBJ) main
