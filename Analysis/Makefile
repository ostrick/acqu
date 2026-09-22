CXX ?= g++
ROOT_CONFIG ?= root-config
CXXFLAGS ?= -O2 -Wall -Wextra
CXXFLAGS += -std=c++11 $(shell $(ROOT_CONFIG) --cflags)
LDLIBS += $(shell $(ROOT_CONFIG) --libs)

TARGETS := analyze_miss_e analyze_pi0_goat

.PHONY: all clean

all: $(TARGETS)

analyze_miss_e: main.cpp AnalyzeMissE.C
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

analyze_pi0_goat: Pi0FromGoAT.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDLIBS)

clean:
	$(RM) $(TARGETS)
