CXXFLAGS=-g -std=c++11
CXX=g++

simulation: simulation.cpp scheduler.cpp event.cpp event.hpp process.cpp process.hpp
	$(CXX) $(CXXFLAGS) simulation.cpp scheduler.cpp event.cpp process.cpp -o sched

clean:
	rm -rf *.o
	rm -f sched
