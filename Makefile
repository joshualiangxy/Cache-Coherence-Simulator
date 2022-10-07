CXX=g++
CXXFLAGS=-g -Wall -std=c++20 -pthread
OBJECTS=Bus.o Cache.o CacheSet.o DragonCacheSet.o MESICacheSet.o Logger.o

all: coherence

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^

coherence: $(OBJECTS) main.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@


clean:
	rm *.o coherence
