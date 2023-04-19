PKGS = gnuradio-runtime gnuradio-blocks spdlog
CXXFLAGS ?= -Wall -Wpedantic -std=c++17 `pkgconf --cflags $(PKGS)`
LDFLAGS ?=  `pkgconf --libs $(PKGS)` -lboost_unit_test_framework
all: doubler_test

doubler_test: doubler_test.o
	$(CXX) -std=c++17 $(LDFLAGS) -o $@ $^

test: doubler_test
	./$<

%.o: %.cc
	$(CXX) -c $(CXXFLAGS) -o $@ $<
