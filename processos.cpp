#include "processos.hpp"

list<Processo*> Processo::processos_lidos;
list<Processo*> Processo::fila_prontos[4];
list<Processo*> Processo::fila_bloqueados;
int Processo::tempo_decorrido = 0;

Processo::~Processo()
{
    Recursos::Desaloca(this->recursos);
    Memoria::Desaloca(this->endereco, this->qtd_blocos);
    this->lista_operacoes.clear();
}

/*Retorna o processo pelo PID ou NULL caso ele nao exista*/
Processo *Processo::Get(int PID)
{
    for(list<Processo*>::iterator it = Processo::processos_lidos.begin(); it != Processo::processos_lidos.end(); it++)
    {
        if((*it)->PID == PID)
        {
            return (*it);
        }
    }
    return NULL;
}

bool Processo::Terminou()
{
    if(!Processo::fila_bloqueados.empty())
    {
        return false;
    }
    else if(!Processo::processos_lidos.empty())
    {
        return false;
    }
    else
    {
        for(int i=0; i<4; i++)
        {
            if(!Processo::fila_prontos[i].empty())
            {
                return false;
            }
        }
    }
    return true;
}

/*Le arquivos de operações (somente parte de operacoes), inserindo as operacoes na lista do processo correspondente e inicializa a memoria*/
void Processo::le_Arquivo_Operacoes(const string &filename)
{
    //le arquivo de operacoes
    ifstream arquivo(filename);
    string linha;
    Operacao operacao;
    int raw_operacoes[5];
    int aux_lines, aux;
    char c, tmp;

    //inicializa memoria
    Memoria::Inicializa();

    if (arquivo.is_open())
    {
        for(int i=0; i<5; i++)
        {
            getline (arquivo,linha);
        }
        while(getline (arquivo,linha))
        {
            istringstream in(linha);

            in >> raw_operacoes[0] >> c;
            in >> raw_operacoes[1] >> c;
            in >> tmp >> c;
            if(raw_operacoes[1] == 0)
            {
                in >> raw_operacoes[3] >> c;
                in >> raw_operacoes[4] >> c;
            }
            else
            {
                in >> raw_operacoes[3] >> c;
            }

            //info basica
            operacao.cod_op = raw_operacoes[1];
            operacao.nome_arquivo = tmp;
            //se operacao de criar
            if(operacao.cod_op == 0)
            {
                operacao.qtd_blocos = raw_operacoes[3];
                operacao.tempo_de_efetivacao = raw_operacoes[4];
            }
            else
            {
                operacao.tempo_de_efetivacao = raw_operacoes[3];
            }
            if(Processo::Get(raw_operacoes[0]) == NULL)
            {
                cout << "\n\tfoi definida operacao no arquivo " << filename << " para processo que nao existe";
                exit(-1);
            }
            Processo::Get(raw_operacoes[0])->lista_operacoes.push_back(operacao);
        }
        arquivo.close();
    }
}

/*Le arquivo de processos e os insere em uma lista de processos lidos*/
void Processo::le_Arquivo_Processo(const string &filename)
{
    ifstream arquivo(filename);
    Processo *processo;
    string linha;
    int raw_processo[8];
    int pid = 0;
    char c;

    if (arquivo.is_open())
    {
        while( getline (arquivo,linha))
        {
            istringstream in(linha);

            for(int i=0; i<8; i++)
            {
                in >> raw_processo[i] >> c;
            }

            processo = new Processo();
            //info basica
            processo->tempo_inicializacao = raw_processo[0];
            processo->prioridade_base = raw_processo[1];
            processo->tempo_processador = raw_processo[2];
            processo->qtd_blocos = raw_processo[3];
            //recursos
            processo->recursos[Processo::scanner] = raw_processo[5];
            processo->recursos[Processo::modem] = raw_processo[6];
            switch(raw_processo[4])
            {
            case 0:
                processo->recursos[Processo::impressora_1] = false;
                processo->recursos[Processo::impressora_2] = false;
                break;
            case 1:
                processo->recursos[Processo::impressora_1] = true;
                processo->recursos[Processo::impressora_2] = false;
                break;
            case 2:
                processo->recursos[Processo::impressora_1] = false;
                processo->recursos[Processo::impressora_2] = true;
                break;
            }
            switch(raw_processo[7])
            {
            case 0:
                processo->recursos[Processo::SATA_1] = false;
                processo->recursos[Processo::SATA_2] = false;
                break;
            case 1:
                processo->recursos[Processo::SATA_1] = true;
                processo->recursos[Processo::SATA_2] = false;
                break;
            case 2:
                processo->recursos[Processo::SATA_1] = false;
                processo->recursos[Processo::SATA_2] = true;
                break;
            }
            //default
            processo->endereco = -1;
            processo->tempo_executado = 0;
            processo->tempo_esperando = 0;
            processo->prioridade_variavel = 3;
            processo->PID = pid++;
            Processo::processos_lidos.push_back(processo);
        }
    }
}

void Processo::imprime_Processo(Processo *processo)
{
    cout << "dispatcher =>";
    cout << "\n\t Hora de criacao:   " << Processo::tempo_decorrido;
    cout << "\n\t PID:               " << processo->PID;
    cout << "\n\t offset:            " << processo->endereco;
    cout << "\n\t blocks:            " << processo->qtd_blocos;
    cout << "\n\t base priority:     " << processo->prioridade_base;
    cout << "\n\t relative priority: " << processo->prioridade_variavel;
    cout << "\n\t tempo:             " << "(" << processo->tempo_executado << " / " << processo->tempo_processador << ")";
    cout << "\n\t - recursos -       ";
    cout << "\n\t impressora 1:      " << processo->recursos[impressora_1];
    cout << "\n\t impressora 2:      " << processo->recursos[impressora_2];
    cout << "\n\t scanner:           " << processo->recursos[scanner];
    cout << "\n\t modem:             " << processo->recursos[modem];
    cout << "\n\t sata 1:            " << processo->recursos[SATA_1];
    cout << "\n\t sata 2:            " << processo->recursos[SATA_2];
    cout << endl << endl;
}

/*Verifica se processo pode alocar os recursos e a memoria necessarios para executar*/
bool Processo::Pode_executar(Processo *processo)
{
    if(!Memoria::Pode_alocar(processo->prioridade_base, processo->qtd_blocos))
    {
        return false;
    }
    if(!Recursos::Pode_alocar(processo->recursos))
    {
        return false;
    }
    return true;
}

/*verifica se o processo executou o tempo maximo determinado*/
bool Processo::terminou()
{
    return this->tempo_executado == this->tempo_processador;
}

/*Move processos de usuario que estao esperando ha um tempo para filas com maior prioridade*/
void Processo::Priority_Boost()
{
    //so pode mover processo de usuario (>0) que nao esta no maior nivel de prioridade variavel
    for(int i=1; i<3; i++)
    {
        for(list<Processo*>::iterator it = Processo::fila_prontos[i].begin(); it != Processo::fila_prontos[i].end(); it++)
        {
            //se processo esta esperando ha muito tempo
            if((*it)->tempo_esperando >= Processo::WAIT_TIME)
            {
                //tira processo da fila atual
                it = Processo::fila_prontos[i].erase(it);
                //coloca processo na fila de maior prioridade
                Processo::fila_prontos[i+1].push_back((*it));
                //reseta tempo de espera do processo movido
                (*it)->tempo_esperando = 0;
            }
        }
    }
}

/*Move processos lidos para as filas de prontos ou bloqueados, se for na de prontos, aloca os recursos necessarios*/
void Processo::Inicializa()
{
    for(list<Processo*>::iterator it = Processo::processos_lidos.begin(); it != Processo::processos_lidos.end(); it++)
    {
        //se esta na hora de inicializar o processo
        if(Processo::tempo_decorrido == (*it)->tempo_inicializacao)
        {
            //se processo tem tudo o que precisa para executar, vai para fila de prontos
            if(Processo::Pode_executar(*it))
            {
                if((*it)->prioridade_base == Processo::TEMPO_REAL)
                {
                    Processo::fila_prontos[0].push_back(*it);
                }
                else
                {
                    Processo::fila_prontos[3].push_back(*it);
                }
                //aloca os recursos e memoria para o processo movido para a fila de prontos
                Recursos::Aloca((*it)->recursos);
                (*it)->endereco = Memoria::Aloca((*it)->prioridade_base, (*it)->qtd_blocos);

                //imprime processo pelo dispatcher apos inicializa-lo
                Processo::imprime_Processo((*it));
            }
            //senao, vai para a fila de bloqueados
            else
            {
                Processo::fila_bloqueados.push_back(*it);
            }
            //tira da fila de processos lidos
            it = Processo::processos_lidos.erase(it);
        }
    }
}

/*Itera sobre a fila de bloqueados, movendo-os para a de prontos caso seja possivel e aloca os recursos ao faze-lo*/
void Processo::Verifica_Bloquados()
{
    Processo::fila_bloqueados.sort();

    for(list<Processo*>::iterator it = Processo::fila_bloqueados.begin(); it != Processo::fila_bloqueados.end(); it++)
    {
        //se processo tem tudo o que precisa para executar, vai para fila de prontos
        if(Processo::Pode_executar(*it))
        {
            if((*it)->prioridade_base == Processo::TEMPO_REAL)
            {
                //insere na fila de prontos
                Processo::fila_prontos[0].push_back(*it);
                //remove da fila de bloqueados
                it = Processo::fila_bloqueados.erase(it);
            }
            else
            {
                //insere na fila de prontos
                Processo::fila_prontos[(*it)->prioridade_variavel].push_back(*it);
                //remove da fila de bloqueados
                it = Processo::fila_bloqueados.erase(it);
            }
            //aloca os recursos e memoria para o processo movido para a fila de prontos
            Recursos::Aloca((*it)->recursos);
            (*it)->endereco = Memoria::Aloca((*it)->prioridade_base, (*it)->qtd_blocos);
        }
    }
}

