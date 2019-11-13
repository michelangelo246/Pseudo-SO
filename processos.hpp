#ifndef PROCESSOS_H
#define PROCESSOS_H

#include <fstream>
#include <string>
#include <iostream>
#include <queue>
#include <list>
#include <sstream>
#include <set>
#include "stdlib.h"
#include "memoria.hpp"
#include "recursos.hpp"

using namespace std;

class Operacao {
public:

    int cod_op; //0: criacao; 1: exclusao;
    char nome_arquivo;
    int qtd_blocos;
    int tempo_de_efetivacao; //hora que a operacao de I/O deve ser realizada.

};

class Processo {
public:
    //informacoes basicas
    int PID;
    int tempo_inicializacao;
    int prioridade_base;
    int prioridade_variavel;
    int tempo_executado;
    int tempo_processador;
    int offset;
    int qtd_blocos;
    int tempo_esperando;

    //recursos solicitados
    static enum { scanner, impressora_1, impressora_2, modem, SATA_1, SATA_2 } kind;
    bool recursos[6];

    //lista de operacoes lidas do arquivo
    list<Operacao> lista_operacoes;

    //misc
    bool terminou();
    ~Processo();

    //Declaracoes estaticas
    static list<Processo *> fila_prontos[4];
    static set<pair<int, Processo*>> fila_bloqueados;
    static list<Processo*> processos_lidos;

    //tempo total decorrido no SO
    static int tempo_decorrido;

    //constantes
    static const int TEMPO_REAL = 0;
    static const int USUARIO = 1;
    static const int QUANTUM = 4;
    static const int WAIT_TIME = 1;

    static Processo *Get(int PID);
    static bool Pode_executar(Processo *processo);
    static void Inicializa();
    static void Verifica_Bloquados();
    static void Priority_Boost();
    static bool Terminou();
    static void imprime_Processo(Processo *processo);
    static void le_Arquivo_Processo(const string &filename);
    static void le_Arquivo_Operacoes(const string &filename);

    /*Definição do operator < para realizar o sort entre processos*/
    friend bool operator<(Processo processo_1, Processo processo_2) {
        if(processo_1.prioridade_base == Processo::TEMPO_REAL) {
            return true;
        } else if(processo_2.prioridade_base == Processo::TEMPO_REAL) {
            return false;
        } else {
            return processo_1.prioridade_variavel < processo_2.prioridade_variavel;
        }
    }
};

#endif
