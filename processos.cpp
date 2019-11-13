#include "processos.hpp"
#include <assert.h>

list<Processo*> Processo::processos_lidos;
list<Processo*> Processo::fila_prontos[4];
set<pair<int, Processo*>> Processo::fila_bloqueados; // usando set para mante-la ordenada pela prioridade
int Processo::tempo_decorrido = 0;

Processo::~Processo() {
    Recursos::Desaloca(this->recursos);
    Memoria::Desaloca(this->offset, this->qtd_blocos);
    this->lista_operacoes.clear();
}

/*Retorna o processo pelo PID ou nullptr caso ele nao exista*/
Processo *Processo::Get(int PID) {
    for(auto process : Processo::processos_lidos)
        if(process->PID == PID)
            return process;
    return nullptr;
}

bool Processo::Terminou() {
    if(not (Processo::fila_bloqueados.empty() and Processo::processos_lidos.empty())) {
        return false;
    } else {
        for(int i=0; i<4; i++) {
            if(!Processo::fila_prontos[i].empty()) {
                return false;
            }
        }
    }
    return true;
}

/*Le arquivos de operações (somente parte de operacoes), inserindo as operacoes na lista do processo correspondente e inicializa a memoria*/
void Processo::le_Arquivo_Operacoes(const string &filename) {
    //le arquivo de operacoes
    ifstream arquivo(filename);
    string linha;
    Operacao operacao;
    int raw_operacoes[5];
    int aux_lines, aux;
    char c, tmp;

    //inicializa memoria
    Memoria::Inicializa();

    if (arquivo.is_open()) {
        for(int i=0; i<5; i++) {
            getline (arquivo,linha);
        }
        while(getline (arquivo,linha)) {
            istringstream in(linha);

            in >> raw_operacoes[0] >> c;
            in >> raw_operacoes[1] >> c;
            in >> tmp >> c;
            if(raw_operacoes[1] == 0) {
                in >> raw_operacoes[3] >> c;
                in >> raw_operacoes[4] >> c;
            } else {
                in >> raw_operacoes[3] >> c;
            }

            //info basica
            operacao.cod_op = raw_operacoes[1];
            operacao.nome_arquivo = tmp;
            //se operacao de criar
            if(operacao.cod_op == 0) {
                operacao.qtd_blocos = raw_operacoes[3];
                operacao.tempo_de_efetivacao = raw_operacoes[4];
            } else {
                operacao.tempo_de_efetivacao = raw_operacoes[3];
            }
            if(Processo::Get(raw_operacoes[0]) == nullptr) {
                cout << "\n\tfoi definida operacao no arquivo " << filename << " para processo que nao existe";
                exit(-1);
            }
            Processo::Get(raw_operacoes[0])->lista_operacoes.push_back(operacao);
        }
        arquivo.close();
    }
}

/*Le arquivo de processos e os insere em uma lista de processos lidos*/
void Processo::le_Arquivo_Processo(const string &filename) {
    ifstream arquivo(filename);
    Processo *processo;
    string linha;
    int raw_processo[8];
    int pid = 0;
    char c;

    if (arquivo.is_open()) {
        while( getline (arquivo,linha)) {
            istringstream in(linha);

            for(int i=0; i<8; i++) {
                in >> raw_processo[i] >> c;
            }

            processo = new Processo();
            //info basica
            processo->tempo_inicializacao = raw_processo[0];
            processo->prioridade_base = raw_processo[1];
            assert(processo->prioridade_base >= 0 and processo->prioridade_base <= 3);
            processo->tempo_processador = raw_processo[2];
            processo->qtd_blocos = raw_processo[3];
            //recursos
            processo->recursos[Processo::scanner] = raw_processo[5];
            processo->recursos[Processo::modem] = raw_processo[6];
            switch(raw_processo[4]) {
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
            switch(raw_processo[7]) {
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
            processo->offset = -1;
            processo->tempo_executado = 0;
            processo->tempo_esperando = 0;
            processo->prioridade_variavel = processo->prioridade_base;
            processo->PID = pid++;
            Processo::processos_lidos.push_back(processo);
        }
    }
}

void Processo::imprime_Processo(Processo *processo) {
    cout << "dispatcher =>";
    cout << "\n\t Hora de criacao:   " << Processo::tempo_decorrido;
    cout << "\n\t PID:               " << processo->PID;
    cout << "\n\t offset:            " << processo->offset;
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
bool Processo::Pode_executar(Processo *processo) {
    return Memoria::Pode_alocar(processo->prioridade_base, processo->qtd_blocos) and Recursos::Pode_alocar(processo->recursos);
}

/*verifica se o processo executou o tempo maximo determinado*/
bool Processo::terminou() {
    return this->tempo_executado == this->tempo_processador;
}

/*Move processos de usuario que estao esperando ha um tempo para filas com maior prioridade*/
void Processo::Priority_Boost() {
    //so pode mover processo de usuario (>1) que nao esta no maior nivel de prioridade variavel
    for(int i=2; i<4; i++) {
        for(auto it = Processo::fila_prontos[i].begin(); it != Processo::fila_prontos[i].end();) {
            auto processo = *it;
            //se processo esta esperando ha muito tempo
            if(processo->tempo_esperando >= Processo::WAIT_TIME) {
                //reseta tempo de espera do processo movido
                processo->tempo_esperando = 0;
                //coloca processo na fila de maior prioridade
                Processo::fila_prontos[i-1].push_back(processo);
                //aumenta campo relativo a prioridade
                processo->prioridade_variavel = i-1;
                //tira processo da fila atual
                it = Processo::fila_prontos[i].erase(it);
            } else {
                //evita incremento duplicado devido ao erase
                it++;
            }
        }
    }
}

/*Move processos lidos para as filas de prontos ou bloqueados, se for na de prontos, aloca os recursos necessarios*/
void Processo::Inicializa() {
    for(auto it = Processo::processos_lidos.begin(); it != Processo::processos_lidos.end();) {
        auto processo = *it;
        //se esta na hora de inicializar o processo
        if(Processo::tempo_decorrido >= processo->tempo_inicializacao) {
            //se processo tem tudo o que precisa para executar, vai para fila de prontos
            if(Processo::Pode_executar(processo)) {
                if(processo->prioridade_base == Processo::TEMPO_REAL) {
                    Processo::fila_prontos[0].push_back(processo);
                    //aloca os recursos e memoria para o processo movido para a fila de prontos
                    Recursos::Aloca(processo->recursos);
                    processo->offset = Memoria::Aloca(processo->prioridade_base, processo->qtd_blocos);

                    //imprime processo pelo dispatcher apos inicializa-lo
                    Processo::imprime_Processo(processo);

                    //tira da fila de processos lidos
                    it = Processo::processos_lidos.erase(it);

                    //inicializar um processo consome um clock do processador
                    Processo::tempo_decorrido++;
                    break;
                } else {
                    Processo::fila_prontos[processo->prioridade_base].push_back(processo);
                }
                //aloca os recursos e memoria para o processo movido para a fila de prontos
                Recursos::Aloca(processo->recursos);
                processo->offset = Memoria::Aloca(processo->prioridade_base, processo->qtd_blocos);

                //imprime processo pelo dispatcher apos inicializa-lo
                Processo::imprime_Processo(processo);
            }
            //senao, vai para a fila de bloqueados se for possivel executar em algum momento
            else if(Memoria::Possivel_alocar(processo->prioridade_base, processo->qtd_blocos)) {
                // 'fila_bloqueados' é um set de pares, portanto ele se mantém ordenado por ordem do primeiro elemento, que é a prioridade
                Processo::fila_bloqueados.emplace(processo->prioridade_base, processo);
                //imprime processo pelo dispatcher apos inicializa-lo
                Processo::imprime_Processo(processo);
            } else {
                cout << "PID " << processo->PID << " requer mais memória do que o sistema possui" << endl;
            }
            //tira da fila de processos lidos
            it = Processo::processos_lidos.erase(it);
            //inicializar um processo consome um clock do processador
            Processo::tempo_decorrido++;
        } else {
            it++;
        }
    }
}

/*Itera sobre a fila de bloqueados, movendo-os para a de prontos caso seja possivel e aloca os recursos ao faze-lo*/
void Processo::Verifica_Bloquados() {
    //ordena por prioridade, pois tem que tirar da fila de bloqueados aquele que tem maior prioridade
    for(auto it = Processo::fila_bloqueados.begin(); it != Processo::fila_bloqueados.end();) {
        auto processo = it->second;
        //se processo tem tudo o que precisa para executar, vai para fila de prontos
        if(Processo::Pode_executar(processo)) {
            //insere na fila de prontos de acordo com a prioridade
            Processo::fila_prontos[processo->prioridade_variavel].push_back(processo);
            //remove da fila de bloqueados
            it = Processo::fila_bloqueados.erase(it);
            //aloca os recursos e memoria para o processo movido para a fila de prontos
            Recursos::Aloca(processo->recursos);
            processo->offset = Memoria::Aloca(processo->prioridade_base, processo->qtd_blocos);
        }
        else{
            it++;
        }
    }
}

