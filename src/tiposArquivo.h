
#include<stdbool.h>

#define TAMANHO_NOME 21
#define VALOR_MAXIMO_CHAVE 100
#define NOME_ARQUIVO_FUNCOES "data_funcoes_hashing.dat"

#define TEST_A -1
#define TEST_B -1

#define INSERE_REGISTROS 'i'
#define CONSULTA_REGISTRO 'c'
#define IMPRIME_NIVEL_UM 'p'
#define IMPRIME_NIVEL_DOIS 's'
#define IMPRIME_ESTRUTURA_GLOBAL 'h'
#define CARDINALIDADE_FUNCOES_HASHING 'n'
#define FIM_DOS_COMANDOS 'e'

typedef struct DadosUsuario DadosUsuario;
typedef struct NivelUm NivelUm;
typedef struct NivelDois NivelDois;
typedef struct Registro Registro;

struct NivelUm{
  int tamanho;
  int primo;
  int a;
  int b;
};
struct NivelDois{
  int tamanho;
  int a;
  int b;
};

struct DadosUsuario{
  int chave;
  char nome[TAMANHO_NOME];
  int idade;
};

struct Registro{
  bool ocupado;
  DadosUsuario dados;
};