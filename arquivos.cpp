#include "arquivos.hpp"
#include "processos.hpp"
#include <string.h>
list<Arquivo*> Arquivo::arquivos;
bool *Arquivo::HD;
int Arquivo::HD_SIZE = 0;

/*Libera memoria alocada por arquivo*/
Arquivo::~Arquivo() {
    //desaloca memoria ocupada pelo arquivo
    memset(HD+this->offset, false, this->qtd_blocos * sizeof(bool));
    //remove arquivo da lista de arquivos
    Arquivo::arquivos.remove(this);
}

/*Le arquivos contendo as operacoes (somente a parte inicial com arquivos iniciais)*/
void Arquivo::le_Arquivo_Operacoes(const string &filename) {
    //le arquivo de operacoes
    ifstream arquivo_entrada(filename);
    Arquivo *arquivo_tmp = nullptr;
    string linha;
    int hd_size, aux_lines, aux;
    char c, tmp;

    if (arquivo_entrada.is_open()) {
        getline(arquivo_entrada, linha);
        istringstream in1(linha);
        in1 >> hd_size;

        getline(arquivo_entrada, linha);
        istringstream in2(linha);
        in2 >> aux_lines;

        for(int i = 0; i < aux_lines; i++) {
            getline(arquivo_entrada, linha);
            istringstream in(linha);
            arquivo_tmp = new Arquivo();

            in >> arquivo_tmp->nome >> c;
            in >> arquivo_tmp->offset >> c;
            in >> arquivo_tmp->qtd_blocos >> c;
            memset(Arquivo::HD + arquivo_tmp->offset, true, arquivo_tmp-> qtd_blocos * sizeof(bool) );
            Arquivo::arquivos.push_back(arquivo_tmp);

            arquivo_tmp = nullptr;
        }
    }
}

/*Exibe estado atual do sistema de arquivos*/
void Arquivo::Imprime() {

    for(int i=0; i<Arquivo::HD_SIZE; i++) {
        if(HD[i] == true) {
            for(auto arquivo : Arquivo::arquivos) {
                if(arquivo->offset == i) {
                    for(int j=0; j<arquivo->qtd_blocos; j++) {
                        cout << arquivo->nome << "|";
                    }
                    i = arquivo->offset + arquivo->qtd_blocos;
                    break;
                }
            }
        } else {
            cout << "_|";
        }
    }
}

/*Inicializa espacos do HD*/
void Arquivo::Inicializa(const string &filename) {
    ifstream arquivo(filename);
    string linha;
    int hd_size;

    getline(arquivo, linha);
    istringstream in1(linha);
    in1 >> hd_size;
    Arquivo::HD_SIZE = hd_size;
    //inicia espacos do HD como livres
    Arquivo::HD = (bool*)calloc(hd_size, sizeof(bool));

}

/*Retorna o arquivo com o nome especificado ou nullptr caso o arquivo nao exista*/
Arquivo* Arquivo::Get(char filename) {
    for(list<Arquivo*>::iterator it = Arquivo::arquivos.begin(); it != Arquivo::arquivos.end(); it++) {
        if((*it)->nome == filename) {
            return (*it);
        }
    }
    return nullptr;
}

/*Executa a operacao no topo da pilha de operacoes do processo*/
void Arquivo::executa(int PID, int cod_op, char nome_arquivo, int qtd_blocos, int tempo_de_efetivacao, int tempo_executado, int prioridade_base) {
    Arquivo *arquivo = nullptr;
    int cabe, aux;

    switch(cod_op) {
    case Arquivo::CRIAR:
        for(int i=0; i<Arquivo::HD_SIZE; i++) {
            //encontrou segmento livre
            if(HD[i] == false) {
                cabe = true;
                //ve se cabe arquivo
                for(int j=0; j<qtd_blocos; j++) {
                    //se nao cabe, continua procurando
                    if((j+i) >= Arquivo::HD_SIZE or HD[j+i] == true) {
                        i = j+i;
                        cabe = false;
                        break;
                    }
                }
                //se percorreu os blocos e cabe
                if(cabe == true) {
                    aux = i;
                    //aloca os espacos e cria o arquivo
                    for(int j=0; j<qtd_blocos; j++)
                        HD[i++] = true;
                    arquivo = new Arquivo();
                    arquivo->nome = nome_arquivo;
                    arquivo->offset = aux;
                    arquivo->qtd_blocos = qtd_blocos;
                    arquivo->PID_owner = PID;
                    Arquivo::arquivos.push_back(arquivo);
                    cout << "PID: " << PID << " - instruction " << tempo_executado << " - SUCCESS (IO)" << endl;
                    cout << "O processo criou o arquivo " << arquivo->nome ;
                    cout << " (blocos " ;
                    for(int k = arquivo->offset; k < arquivo->offset + arquivo->qtd_blocos; k++) {
                        cout << k;
                        if(k < arquivo->offset + arquivo->qtd_blocos-1)
                            cout << " e ";
                    }
                    cout << ")" << endl << endl;
                    break;
                }
            }
        }
        if(cabe == false) {
            cout << "PID: " << PID << " - instruction " << tempo_executado << " - FAIL (IO)" << endl;
            cout << "O processo nao pode criar o arquivo ";
            cout << nome_arquivo << " por falta de espaco" << endl << endl;
        }
        break;
    case Arquivo::EXCLUIR:
        //Se arquivo nao existe
        if(not Arquivo::Get(nome_arquivo)) {
            cout << "PID: " << PID << " - instruction " << tempo_executado << " - FAIL (IO)" << endl;
            cout << "O processo nao pode deletar o arquivo ";
            cout << nome_arquivo << " porque nao existe esse arquivo" << endl << endl;
        } else {
            //processo de tempo real pode excluir qualquer arquivo
            //processo dono pode excluir o arquivo
            if((prioridade_base == Processo::TEMPO_REAL) or (PID == Arquivo::Get(nome_arquivo)->PID_owner)) {
                delete Arquivo::Get(nome_arquivo);
                cout << "PID: " << PID << " - instruction " << tempo_executado << " - SUCCESS (IO)" << endl;
                cout << "O processo deletou o arquivo ";
                cout << nome_arquivo << endl << endl;
            } else {
                cout << "PID: " << PID << " - instruction " << tempo_executado << " - FAIL (IO)" << endl;
                cout << "O processo nao pode deletar o arquivo ";
                cout << nome_arquivo << " pois nao o criou" << endl << endl;
            }
        }
        break;
    }
}

void Arquivo::Free() {
    free(HD);
    HD = nullptr;
}
