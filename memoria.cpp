#include "memoria.hpp"

bool Memoria::RAM[1024];

/*Inicializa os blocos da memoria RAM como livres*/
void Memoria::Inicializa()
{
    for(int i=0; i<1024; i++)
    {
        Memoria::RAM[i]=false;
    }
}

/*verifica se ha a possibilidade de alocar a quantidade de blocos solicitadas no segmento da prioridade informada*/
bool Memoria::Pode_alocar(int prioridade, int qtd_blocos)
{
    bool cabe;

    if(prioridade == TEMPO_REAL)
    {
        for(int i=0; i<USER_OFFSET; i++)
        {
            //encontrou segmento livre
            if(Memoria::RAM[i] == false)
            {
                cabe = true;
                //ve se cabe processo
                for(int j=0; j<qtd_blocos; j++)
                {
                    //se nao cabe, continua procurando
                    if(Memoria::RAM[j+i] == true || (j+i) >= USER_OFFSET)
                    {
                        i = j+i;
                        cabe = false;
                        break;
                    }
                }
                //se percorreu os blocos e cabe
                if(cabe == true)
                {
                    return true;
                }
            }
        }
    }
    else
    {
        for(int i=USER_OFFSET; i<1024; i++)
        {
            //encontrou segmento livre
            if(Memoria::RAM[i] == false)
            {
                cabe = true;
                //ve se cabe processo
                for(int j=0; j<qtd_blocos; j++)
                {
                    //se nao cabe, continua procurando
                    if(Memoria::RAM[j+i] == true || (j+i)>=1024)
                    {
                        i = j+i;
                        cabe = false;
                        break;
                    }
                }
                //se percorreu os blocos e cabe
                if(cabe == true)
                {
                    return true;
                }
            }
        }
    }
    return false;
}

/*Aloca a quantidade de blocos especificada no segmento da prioridade e retorna o offset da posicao de alocacao*/
int Memoria::Aloca(int prioridade, int qtd_blocos)
{
    bool cabe;
    int aux;

    if(prioridade == TEMPO_REAL)
    {
        for(int i=0; i<USER_OFFSET; i++)
        {
            //encontrou bloco livre
            if(Memoria::RAM[i] == false)
            {
                cabe = true;
                //ve se cabe processo
                for(int j=0; j<qtd_blocos; j++)
                {
                    //se nao cabe, continua procurando
                    if(Memoria::RAM[j+i] == true || (j+i) >= USER_OFFSET)
                    {
                        i = j+i;
                        cabe = false;
                        break;
                    }
                }
                //se percorreu os blocos e cabe
                if(cabe == true)
                {
                    aux = i;
                    //aloca os espacos e retorna o offset
                    for(int j=0; j<qtd_blocos; j++)
                    {
                        Memoria::RAM[i++] = true;
                    }
                    return aux;
                }
            }
        }
    }
    else
    {
        for(int i=USER_OFFSET; i<1024; i++)
        {
            //encontrou segmento livre
            if(Memoria::RAM[i] == false)
            {
                cabe = true;
                //ve se cabe processo
                for(int j=0; j<qtd_blocos; j++)
                {
                    //se nao cabe, continua procurando
                    if(Memoria::RAM[j+i] == true || (j+i)>=1024)
                    {
                        i = j+i;
                        cabe = false;
                        break;
                    }
                }
                //se percorreu os blocos e cabe
                if(cabe == true)
                {
                    aux = i;
                    //aloca os espacos e retorna o offset
                    for(int j=0; j<qtd_blocos; j++)
                    {
                        Memoria::RAM[i++] = true;
                    }
                    return aux;
                }
            }
        }
    }
}

/*Libera a memoria na quantidade de blocos especificada a partir do offset informado*/
void Memoria::Desaloca(int offset, int qtd_blocos)
{
    for(int i=offset; i<qtd_blocos; i++)
    {
        Memoria::RAM[i]=false;
    }
}
