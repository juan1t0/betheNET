peer2.exe: peer2.cpp lib/cliser.o
	g++ -std=c++11 peer2.cpp cliser.o -o peer2.exe -lpthread

tracker2.exe: tracker2.cpp lib/cliser.o
	g++ -std=c++11 tracker2.cpp cliser.o -o tracker2.exe -lpthread

lib/cliser.o: lib/cliser.cpp
	g++ -c -std=c++11 lib/cliser.cpp -o cliser.o -lpthread
