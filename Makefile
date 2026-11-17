CXX ?= g++
CXXFLAGS := -g -std=c++17 -Wall -Wextra -Iinclude -Isrc/generated -Ithird_party -MMD -MP
LDLIBS := -lglut -lGL -lGLU -lGLEW

SOURCES := $(wildcard src/*.cpp)
OBJECTS := $(patsubst src/%.cpp,build/%.o,$(SOURCES))
TARGET := build/phm-scene

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDLIBS)

build/%.o: src/%.cpp
	@mkdir -p build
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build

-include $(OBJECTS:.o=.d)
