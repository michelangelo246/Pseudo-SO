#ifndef ARQUIVOS_H
#define ARQUIVOS_H

#include <fstream>
#include <string>
#include <iostream>
#include <queue>
#include <list>
#include <sstream>
#include "stdlib.h"

using namespace std;

class Arquivo {
public:

    char nome;
    int offset;
    int qtd_blocos;
    int PID_owner;

    ~Arquivo();

    static list<Arquivo*> arquivos;
    static char *HD;
    static int HD_SIZE;

    static const int CRIAR = 0;
    static const int EXCLUIR = 1;
    static const char VAZIO = '_';

    static void executa(int PID,int cod_op, char nome_arquivo, int qtd_blocos, int tempo_de_efetivacao, int tempo_executado, int prioridade_base);
    static void Inicializa(const string &filename);
    static void Imprime();
    static void le_Arquivo_Operacoes(const string &filename);
    static void Free();
    static Arquivo* Get(char filename);

};

#endif
