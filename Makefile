CXX = clang++
CXXFLAGS = -std=c++11 -Wall -Wextra -pedantic -O2 -I include

TARGETS = SOAContainerTest

all: $(TARGETS)

clean:
	rm -fr $(TARGETS) *.o

tests: all
	./SOAContainerTest

doxy:
	doxygen

SOAContainerTest.o: tests/SOAContainerTest.cc include/SOAContainer.h \
    include/SOAIterator.h include/SOATypelist.h include/SOATypelistUtils.h \
    include/SOAUtils.h

SOAContainerTest: SOAContainerTest.o

SOAContainerTest: CC=$(CXX)

%.o: tests/%.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $<
