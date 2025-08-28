# компилятор
CXX := g++

# BUILD может быть debug или release (по умолчанию debug)
BUILD ?= debug

# флаги для двух режимов
ifeq ($(BUILD),release)
  CXXFLAGS := -std=c++17 -O3 -DNDEBUG -Wall -Wextra \
              -pthread $(shell mysql_config --cflags) \
              -L/usr/lib64/firebird -lfbclient -lcurl
  LDFLAGS  := -pthread $(shell mysql_config --libs) -lstdc++fs -s
else
  CXXFLAGS := -std=c++17 -Wall -Werror -fsanitize=address \
              -fdiagnostics-color=always -g -Og -O0 \
              -pthread $(shell mysql_config --cflags) \
              -L/usr/lib64/firebird -lfbclient -lcurl
  LDFLAGS  := -pthread $(shell mysql_config --libs) -lstdc++fs
endif

# источники и объекты
SRC_ROOT := $(wildcard *.cpp)
SRC_AST  := $(wildcard clearing_current_day/*.cpp different_checks/*.cpp)
SOURCES  := $(SRC_ROOT) $(SRC_AST)
OBJECTS  := $(SOURCES:.cpp=.o)
TARGET   := core_dashboard

.PHONY: all release clean

all: $(TARGET)

release: BUILD := release
release: clean all
	@echo "Собран релиз: $(TARGET)"

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)