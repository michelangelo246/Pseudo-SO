#include "memoria.hpp"
#include <string.h>

bool Memoria::RAM[TAM_MEMORIA];
int Memoria::candidato_alocacao = -1;

/*Inicializa os blocos da memoria RAM como livres*/
void Memoria::Inicializa() {
    memset(Memoria::RAM, false, sizeof Memoria::RAM);
}

/*verifica se em algum momento vai ser possivel alocar a quantidade de blocos solicitados no segmento da prioridade informada */
bool Memoria::Possivel_alocar(int prioridade, int qtd_blocos) {
    return qtd_blocos <= (prioridade==0 ? USER_OFFSET : TAM_MEMORIA - USER_OFFSET);
}
/*verifica se ha a possibilidade de alocar a quantidade de blocos solicitadas no segmento da prioridade informada */
bool Memoria::Pode_alocar(int prioridade, int qtd_blocos) {
    bool cabe;

    // determina o inicio e o fim da região de memoria para o processo
    int inicio = 0, fim = TAM_MEMORIA;
    if(prioridade == TEMPO_REAL) fim = USER_OFFSET;
    else inicio = USER_OFFSET;

    for(int i = inicio; i < fim; i++) {
        //encontrou segmento livre
        if(Memoria::RAM[i] == false) {
            cabe = true;
            //ve se cabe processo
            for(int j = 0; j < qtd_blocos; j++) {
                //se nao cabe, continua procurando
                if((j+i) >= fim or Memoria::RAM[j+i]) {
                    i = j+i;
                    cabe = false;
                    break;
                }
            }
            //se percorreu os blocos e cabe
            if(cabe) {
                Memoria::candidato_alocacao = i;
                return true;
            }
        }
    }
    return false;
}

/*Aloca a quantidade de blocos especificada no segmento da prioridade e retorna o offset da posicao de alocacao*/
int Memoria::Aloca(int prioridade, int qtd_blocos) {
    memset(Memoria::RAM + Memoria::candidato_alocacao, true, qtd_blocos * sizeof(bool));
    return Memoria::candidato_alocacao;
}

/*Libera a memoria na quantidade de blocos especificada a partir do offset informado*/
void Memoria::Desaloca(int offset, int qtd_blocos) {
    memset(Memoria::RAM + offset, false, qtd_blocos * sizeof(bool));
}
