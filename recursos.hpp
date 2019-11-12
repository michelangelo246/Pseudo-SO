#ifndef RECURSOS_H
#define RECURSOS_H

#include <fstream>
#include <string>
#include <iostream>
#include <queue>
#include <list>

using namespace std;

class Recursos {
public:

    static enum { scanner_1, scanner_2, impressora_1, impressora_2, modem, SATA_1, SATA_2 } kind;
    static bool recursos[7];

    static bool Pode_alocar(bool recursos[7]);
    static void Aloca(bool recursos[7]);
    static void Desaloca(bool recursos[7]);

};

#endif
