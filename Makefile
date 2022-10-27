CXX=g++
CXXFLAGS=-g -Wall -std=c++2a -pthread
OBJECTS=Bus.o Cache.o CacheSet.o DragonCacheSet.o MESICacheSet.o Logger.o MESIFCacheSet.o

all: coherence

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $^

coherence: $(OBJECTS) main.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@


clean:
	rm *.o coherence
