#ifndef MEMORIA_H
#define MEMORIA_H

#include <fstream>
#include <string>
#include <iostream>
#include <queue>
#include <list>

using namespace std;

class Memoria
{
public:

    static bool RAM[1024];

    static const int USER_OFFSET = 64;
    static const int TEMPO_REAL = 0;
    static const int USUARIO = 1;

    static void Inicializa();
    static bool Pode_alocar(int prioridade, int qtd_blocos);
    static int Aloca(int prioridade, int qtd_blocos);
    static void Desaloca(int offset, int qtd_blocos);

};

#endif
