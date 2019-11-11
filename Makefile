all: main.o recursos.o processos.o memoria.o arquivos.o
	g++ -o prog.exe main.o recursos.o processos.o memoria.o arquivos.o
	rm *.o

main.o: main.cpp main.hpp
	g++ -g -c main.cpp

recursos.o: recursos.cpp recursos.hpp
	g++ -g -c recursos.cpp

processos.o: processos.cpp processos.hpp
	g++ -g -c processos.cpp

memoria.o: memoria.cpp memoria.hpp
	g++ -g -c memoria.cpp

arquivos.o: arquivos.cpp arquivos.hpp
	g++ -g -c arquivos.cpp
