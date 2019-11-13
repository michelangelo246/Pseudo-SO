all: main.o recursos.o processos.o memoria.o arquivos.o
	g++ -std=c++11 -o prog.exe main.o recursos.o processos.o memoria.o arquivos.o
	rm *.o

main.o: main.cpp main.hpp
	g++ -std=c++11 -g -c main.cpp

recursos.o: recursos.cpp recursos.hpp
	g++ -std=c++11 -g -c recursos.cpp

processos.o: processos.cpp processos.hpp
	g++ -std=c++11 -g -c processos.cpp

memoria.o: memoria.cpp memoria.hpp
	g++ -std=c++11 -g -c memoria.cpp

arquivos.o: arquivos.cpp arquivos.hpp
	g++ -std=c++11 -g -c arquivos.cpp
