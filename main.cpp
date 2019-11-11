#include "main.hpp"

using namespace std;

/*Executa uma operacao da lista de operacoes e incrementa tempo de execucao*/
void executa_prox_op(Processo *processo)
{
    //se ha operacoes de E/S a serem executadas
    if(!processo->lista_operacoes.empty())
    {
        //se esta na hora de executar a prox operacao
        if(processo->tempo_executado >= processo->lista_operacoes.front().tempo_de_efetivacao)
        {
            Arquivo::executa(processo->PID,processo->lista_operacoes.front().cod_op,
                    processo->lista_operacoes.front().nome_arquivo, processo->lista_operacoes.front().qtd_blocos,
                    processo->lista_operacoes.front().tempo_de_efetivacao, processo->tempo_executado,
                    processo->prioridade_base);
            processo->lista_operacoes.pop_front();
        }
        else
        {
            cout << "PID: " << processo->PID << " - instruction " << processo->tempo_executado << " - SUCCESS (CPU)"<< endl << endl;
        }
    }
    else
    {
        cout << "PID: " << processo->PID << " - instruction " << processo->tempo_executado << " - SUCCESS (CPU)"<< endl << endl;
    }

    //incrementa tempo de execucao
    processo->tempo_executado++;
}

/*percorre filas de prontos, executa a operacao daquele que possui maior prioridade e o insere no final da fila*/
void Executa()
{
    Processo *processo;

    /*Percorre as filas de prontos seguindo a prioridade*/
    for(int i=0; i<4; i++)
    {
        if(!Processo::fila_prontos[i].empty())
        {
            //tira da fila de pronto
            processo = Processo::fila_prontos[i].front();
            Processo::fila_prontos[i].pop_front();

            //reseta tempo de espera
            processo->tempo_esperando = 0;

            //aumenta tempo de espera de todos processos prontos que nao executaram
            //Processo::Priority_Boost();

            //executa operacoes; se for de tempo real, todas; se for de usuario, ate o quantum acabar;
            if(processo->prioridade_base == Processo::TEMPO_REAL)
            {
                while(!processo->terminou())
                {
                    executa_prox_op(processo);
                }
            }
            else
            {
                for(int j=0; (j<Processo::QUANTUM)&&(!processo->terminou()); j++)
                {
                    executa_prox_op(processo);
                }
            }

            //se processo ainda nao terminou, devolve no final da fila de prontos
            if(processo->terminou())
            {
                //informa que as operacoes especificadas para execucao apos encerramento serao abortadas
                cout << "PID: " << processo->PID << "  - TERMINATED " ;
                if(!processo->lista_operacoes.empty())
                {
                    cout << "\nSuas operacoes restantes foram abortadas!" << endl << endl;
                }
                else
                {
                    cout << "\nTodas as suas operacoes foram executadas!" << endl << endl;
                }
                delete(processo);
            }
            else
            {
                cout << "PID: " << processo->PID << " - instruction " << processo->tempo_executado << " - FAIL";
                cout << "\nO quantum de processamento foi esgotado!" << endl << endl;
                Processo::fila_prontos[i].push_back(processo);
            }
            break;
        }
    }
}

int main(int argc, char **argv)
{
    Processo::le_Arquivo_Processo(argv[1]);
    Arquivo::Inicializa(argv[2]);
    Processo::le_Arquivo_Operacoes(argv[2]);

    //loop principal: 
    // 1. inicializa processos caso seja a hora
    // 2. move processos bloquados para a fila de prontos caso possivel
    // 3. executa processo pronto de maior prioridade
    // 4. envia processo de usuario para o fim da mesma fila apos executar
    // 5. aumenta a prioridade dos processos que nao conseguiram executar
    do{
        Processo::Inicializa();
        Processo::Verifica_Bloquados();
        Executa();
        Processo::tempo_decorrido++;
    }
    while(!Processo::Terminou());

    //Exibe estado final do sistema de arquivos
    Arquivo::Imprime();

    return 0;
}