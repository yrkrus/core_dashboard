# компилятор
CXX      := g++
# по умолчанию – debug
BUILD    ?= debug

# все в лог!! очень много лога !!!
CREATE_LOG_DEBUG := 0
export CREATE_LOG_DEBUG

# если в списке целей есть одна из release|debug|valgrind,
# то перезапишем BUILD
ifneq ($(filter release,$(MAKECMDGOALS)),)
  BUILD := release
endif
ifneq ($(filter debug,$(MAKECMDGOALS)),)
  BUILD := debug
endif
ifneq ($(filter valgrind,$(MAKECMDGOALS)),)
  BUILD := valgrind
endif

# выставляем флаги в зависимости от BUILD
ifeq ($(BUILD),release)
  CXXFLAGS := -std=c++17 -O2 -DNDEBUG -Wall -Wextra \
              -pthread $(shell mysql_config --cflags) \
              -L/usr/lib64/firebird -lfbclient -lcurl
  LDFLAGS  := -pthread $(shell mysql_config --libs) -lstdc++fs -s
else ifeq ($(BUILD),valgrind)
  CXXFLAGS := -std=c++17 -g -O0 -Wall \
              -pthread $(shell mysql_config --cflags) \
              $(if $(CREATE_LOG_DEBUG))
              -L/usr/lib64/firebird -lfbclient -lcurl
  LDFLAGS  := -pthread $(shell mysql_config --libs) -lstdc++fs
else
#   # debug с ASan
#   CXXFLAGS := -std=c++17 -Wall -Werror -fsanitize=address \
#               -fdiagnostics-color=always -g -Og \
#               -pthread $(shell mysql_config --cflags) \
#               -L/usr/lib64/firebird -lfbclient -lcurl
#   LDFLAGS  := -pthread $(shell mysql_config --libs) -lstdc++fs -fsanitize=address

CXXFLAGS := -std=c++17 -Wall -Werror -g -O0                                     \
            -fno-inline -fno-inline-functions -fno-optimize-sibling-calls       \
            -fno-omit-frame-pointer  -fdiagnostics-color=always                 \
            -pthread $(shell mysql_config --cflags)                             \
            -L/usr/lib64/firebird -lfbclient -lcurl

# Добавляем флаг для логирования, если он включен
  ifeq ($(CREATE_LOG_DEBUG),1)
    CXXFLAGS += -DCREATE_LOG_DEBUG
  endif

LDFLAGS  := -pthread $(shell mysql_config --libs) -lstdc++fs
endif

# источники
SRC_ROOT := $(wildcard *.cpp)
SRC_AST  := $(wildcard clearing_current_day/*.cpp core/*.cpp different_checks/*.cpp interfaces/*.cpp system/*.cpp utils/*.cpp)
SOURCES  := $(SRC_ROOT) $(SRC_AST)
OBJECTS  := $(SOURCES:.cpp=.o)
TARGET   := core_dashboard

.PHONY: all release debug valgrind clean

# цель по умолчанию
all: $(TARGET)

# когда вызываем make release/debug/valgrind,
# сначала выполнится clean, затем сборка, затем echo
release debug valgrind: clean all
	@echo "============================== Сборка -> $(BUILD): $(TARGET) =============================="

# линковка
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# компиляция каждого .cpp
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(TARGET)