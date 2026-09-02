CXX ?= g++
ROOT_CONFIG ?= root-config
CXXFLAGS ?= -O2 -Wall -Wextra
CXXFLAGS += -std=c++11 $(shell $(ROOT_CONFIG) --cflags)
LDLIBS += $(shell $(ROOT_CONFIG) --libs)

TARGET := analyze_miss_e
SOURCES := main.cpp AnalyzeMissE.C

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $@ $(LDLIBS)

clean:
	$(RM) $(TARGET)
