# компилятор
CXX       := g++
#CXXVERSION := $(shell g++ -dumpversion | sed -E 's/^([0-9]+)\..*/\1/')

# флаги для компиляции
CXXFLAGS  := -std=c++17 -Wall -Werror -fsanitize=address -g -Og -O0 \
              -pthread $(shell mysql_config --cflags) \
			  -L/usr/lib64/firebird -lfbclient

# флаги для линковки
LDFLAGS   := -pthread $(shell mysql_config --libs) -lstdc++fs

# исходники
SRC_ROOT  := $(wildcard *.cpp)
SRC_AST   := $(wildcard clearing_current_day/*.cpp)
SOURCES   := $(SRC_ROOT) $(SRC_AST)
OBJECTS   := $(SOURCES:.cpp=.o)
TARGET    := core_dashboard

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)