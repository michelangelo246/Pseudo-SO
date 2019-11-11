#include "arquivos.hpp"

list<Arquivo*> Arquivo::arquivos;
bool *Arquivo::HD;
int Arquivo::HD_SIZE = 0;

/*Libera memoria alocada por arquivo*/
Arquivo::~Arquivo()
{
    //desaloca memoria ocupada pelo arquivo
    for(int i=this->offset; i<this->qtd_blocos; i++)
    {
        HD[i]=false;
    }
    //remove arquivo da lista de arquivos
    Arquivo::arquivos.remove(this);
}

void Arquivo::le_Arquivo_Operacoes(const string &filename)
{
    //le arquivo de operacoes
    ifstream arquivo(filename);
    Arquivo *arquivo_tmp;
    string linha;
    int hd_size, aux_lines, aux, aux_offset;
    char c, tmp;

    if (arquivo.is_open())
    {
        getline(arquivo, linha);
        istringstream in1(linha);
        in1 >> hd_size;

        getline(arquivo, linha);
        istringstream in2(linha);
        in2 >> aux_lines;

        for(int i=0; i<aux_lines; i++)
        {
            getline(arquivo, linha);
            istringstream in(linha);
            arquivo_tmp = new Arquivo();

            in >> tmp >> c;
            arquivo_tmp->nome = tmp;
            in >> aux_offset >> c;
            arquivo_tmp->offset = aux_offset;
            in >> aux >> c;
            arquivo_tmp->qtd_blocos = aux;

            for(int j=0; j<arquivo_tmp->qtd_blocos; j++)
            {
                Arquivo::HD[aux_offset++] = true;
            }
            Arquivo::arquivos.push_back(arquivo_tmp);
        }
        arquivo.close();
    }
}

void Arquivo::Imprime()
{

    for(int i=0; i<Arquivo::HD_SIZE; i++)
    {
        if(HD[i] == true)
        {
            for(list<Arquivo*>::iterator it = Arquivo::arquivos.begin(); it != Arquivo::arquivos.end(); it++)
            {
                if((*it)->offset == i)
                {
                    for(int j=0; j<(*it)->qtd_blocos; j++)
                    {
                        cout << (*it)->nome << "|";
                    }
                    break;
                    i = (*it)->offset + (*it)->qtd_blocos;
                }
            }
        }
        else
        {
            cout << "_|";
        }
    }
}

/*Inicializa espacos do HD*/
void Arquivo::Inicializa(const string &filename)
{
    ifstream arquivo(filename);
    string linha;
    int hd_size;

    getline(arquivo, linha);
    istringstream in1(linha);
    in1 >> hd_size;
    Arquivo::HD_SIZE = hd_size;
    Arquivo::HD = (bool*)malloc(hd_size*sizeof(bool));

    //inicia espacos do HD como livres
    for(int i=0; i<Arquivo::HD_SIZE; i++)
    {
        HD[i] = false;
    }

    Arquivo::le_Arquivo_Operacoes(filename);
}

/*Retorna o arquivo com o nome especificado ou NULL caso o arquivo nao exista*/
Arquivo* Arquivo::Get(char filename)
{
    for(list<Arquivo*>::iterator it = Arquivo::arquivos.begin(); it != Arquivo::arquivos.end(); it++)
    {
        if((*it)->nome == filename)
        {
            return (*it);
        }
    }
    return NULL;
}

/*Executa a operacao no topo da pilha de operacoes do processo*/
void Arquivo::executa(int PID,int cod_op, char nome_arquivo, int qtd_blocos, int tempo_de_efetivacao, int tempo_executado, int prioridade_base)
{
    Arquivo *arquivo;
    int cabe,aux;

    switch(cod_op)
    {
    case Arquivo::CRIAR:
        for(int i=0; i<Arquivo::HD_SIZE; i++)
        {
            //encontrou segmento livre
            if(HD[i] == false)
            {
                cabe = true;
                //ve se cabe arquivo
                for(int j=0; j<qtd_blocos; j++)
                {
                    //se nao cabe, continua procurando
                    if(HD[j+i] == true || (j+i) > HD_SIZE)
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
                    //aloca os espacos e cria o arquivo
                    for(int j=0; j<qtd_blocos; j++)
                    {
                        HD[i++] = true;
                    }
                    arquivo = new Arquivo();
                    arquivo->nome = nome_arquivo;
                    arquivo->offset = aux;
                    arquivo->qtd_blocos = qtd_blocos;
                    Arquivo::arquivos.push_back(arquivo);
                    cout << "PID: " << PID << " - instruction " << tempo_executado << " - SUCCESS (IO)" << endl;
                    cout << "O processo criou o arquivo " << arquivo->nome ;
                    cout << " (blocos " ;
                    for(int k=arquivo->offset;k< arquivo->offset+arquivo->qtd_blocos; k++)
                    {
                        cout << k;
                        if(k < arquivo->offset+arquivo->qtd_blocos-1)
                        {
                            cout << " e ";
                        }
                    }
                    cout << ")" << endl << endl;
                    break;
                }
            }
        }
        if(cabe == false)
        {
            cout << "PID: " << PID << " - instruction " << tempo_executado << " - FAIL (IO)" << endl;
            cout << "O processo nao pode criar o arquivo ";
            cout << nome_arquivo << " por falta de espaco" << endl << endl;
        }
        break;
    case Arquivo::EXCLUIR:
        //Se arquivo nao existe
        if(!Arquivo::Get(nome_arquivo))
        {
            cout << "PID: " << PID << " - instruction " << tempo_executado << " - FAIL (IO)" << endl;
            cout << "O processo nao pode deletar o arquivo ";
            cout << nome_arquivo << " porque nao existe esse arquivo" << endl << endl;;
        }
        else
        {
            //processo de tempo real pode excluir qualquer arquivo
            if(prioridade_base == 0)
            {
                delete(Arquivo::Get(nome_arquivo));
                cout << "PID: " << PID << " - instruction " << tempo_executado << " - SUCCESS (IO)" << endl;
                cout << "O processo deletou o arquivo ";
                cout << nome_arquivo << endl << endl;
            }
            else
            {
                //se processo nao e owner do arquivo
                if(PID != Arquivo::Get(nome_arquivo)->PID_owner)
                {
                    cout << "PID: " << PID << " - instruction " << tempo_executado << " - FAIL (IO)" << endl;
                    cout << "O processo nao pode deletar o arquivo ";
                    cout << nome_arquivo << " pois nao o criou" << endl << endl;
                }
                else
                {
                    delete(Arquivo::Get(nome_arquivo));
                    cout << "PID: " << PID << " - instruction " << tempo_executado << " - SUCCESS (IO)" << endl;
                    cout << "O processo deletou o arquivo ";
                    cout << nome_arquivo << endl << endl;
                }
            }
        }
        break;
    }
}
