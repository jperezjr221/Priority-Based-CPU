all: priority

priority: Priority-BasedCPU.cpp

	g++ -std=c++11 -pthread -o priority Priority-BasedCPU.cpp

clean:

	rm -f priority
