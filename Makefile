CXX       := g++
CXXFLAGS  := -std=c++17 -Wall -I. -I./clearing_current_day -pthread -g -Og
MYSQL_CFLAGS := $(shell mysql_config --cflags)
MYSQL_LIBS   := $(shell mysql_config --libs)

SRC_ROOT  := $(wildcard *.cpp)
SRC_AST   := $(wildcard clearing_current_day/*.cpp)
SOURCES   := $(SRC_ROOT) $(SRC_AST)
OBJECTS   := $(SOURCES:.cpp=.o)
TARGET    := core_dashboard

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(MYSQL_LIBS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)