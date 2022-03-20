CXXFLAGS=-g -std=c++11
CXX=g++

simulation: simulation.cpp scheduler.cpp event.hpp process.hpp
	$(CXX) $(CXXFLAGS) simulation.cpp scheduler.cpp -o simulation

clean:
	rm -rf *.o
	rm -f simulation
