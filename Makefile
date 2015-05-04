CXX = clang++
CXXFLAGS = -g -std=c++11 -Wall -Wextra -pedantic -march=native -O2 -ftree-vectorize -I include

TARGETS = SOAContainerTest

all: $(TARGETS)

clean:
	rm -fr $(TARGETS) *.o

tests: all
	./SOAContainerTest

doxy:
	doxygen

SOAContainerTest.o: tests/SOAContainerTest.cc include/SOAContainer.h \
    include/SOAObjectProxy.h include/SOAIterator.h include/SOATypelist.h \
    include/SOATypelistUtils.h include/SOAUtils.h

SOAContainerTest: SOAContainerTest.o

SOAContainerTest: CC=$(CXX)
SOAContainerTest: LDFLAGS=$(CXXFLAGS)

%.o: tests/%.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<
