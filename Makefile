CXX=g++
CXXFLAGS=-g -Wall -std=c++20 -pthread
OBJECTS=Cache.o DragonCache.o MESICache.o

all: coherence

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^

coherence: $(OBJECTS) main.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@


clean:
	rm *.o coherence
