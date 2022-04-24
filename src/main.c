#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define TAMANHO_NOME 20
#define VALOR_MAXIMO_CHAVE 100
#define NOME_ARQUIVO_FUNCOES "data_funcoes_hashing.bin"

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

void insere_registros();
void consulta_registro();
void imprime_nivel_um();
void imprime_nivel_dois();
void imprime_estrutura_global();
void cardinalidade_funcoes_hashing();
void cria_arquivo_de_funcoes(int tamanho, int primo);
void insere_registro(Registro* registro);

int proximo_primo(int n);
int get_numero_aleatorio_entre(int min, int max);
int hash(int p, int m, int a, int b, int k);
char* get_nome_arquivo_registros(int posicao);
char* get_nome_arquivo_registros_temp(int posicao);

FILE *abre_arquivo(char *nomeArquivo, char *modo);
FILE* cria_arquivo_de_registros(char* nome, int tamanho);

int main() {
  srand(time(NULL));
  
  char comando;
  while (scanf("%s", &comando)) {
    switch (comando) {
      case INSERE_REGISTROS:
        insere_registros();
        break;
      case CONSULTA_REGISTRO:
        consulta_registro();
        break;
      case IMPRIME_NIVEL_UM:
        imprime_nivel_um();
        break;
      case IMPRIME_NIVEL_DOIS:
        imprime_nivel_dois();
        break;
      case IMPRIME_ESTRUTURA_GLOBAL:
        imprime_estrutura_global();
        break;
      case CARDINALIDADE_FUNCOES_HASHING:
        cardinalidade_funcoes_hashing();
        break;
      case FIM_DOS_COMANDOS:
        return 0;
      default:
        printf("Comando inválido\n");
        return 1;
    }
  }
  return 0;
}

void cria_arquivo_de_funcoes(int tamanho, int primo){
  FILE *arquivo;
  // if (arquivo = fopen(NOME_ARQUIVO_FUNCOES, "r")) {
  //   fclose(arquivo);
  //   return;
  // }

  arquivo = abre_arquivo(NOME_ARQUIVO_FUNCOES, "w+");

  NivelUm nivelUm;
  nivelUm.tamanho = tamanho;
  nivelUm.primo = primo;
  nivelUm.a = get_numero_aleatorio_entre(0, primo);
  nivelUm.b = get_numero_aleatorio_entre(1, primo);

  fwrite(&nivelUm, sizeof(NivelUm), 1, arquivo);

  int i;
  NivelDois nivelDois;
  nivelDois.tamanho = 0;
  nivelDois.a = 0;
  nivelDois.b = 0;
  
  for (i = 0; i < tamanho; i++) {
    fwrite(&nivelDois, sizeof(NivelDois), 1, arquivo);
  }
  fclose(arquivo);
}

void insere_registros() {
  int tamanho;
  scanf("%d", &tamanho);

  int i;
  for (i = 0; i < tamanho; i++) {
    Registro novoRegistro;
    novoRegistro.ocupado = true;
    scanf("%d", &novoRegistro.dados.chave);
    if (i == 0) {
      int primo = proximo_primo(novoRegistro.dados.chave);
      cria_arquivo_de_funcoes(tamanho, primo);
    }
    
    scanf("%s", novoRegistro.dados.nome);
    scanf("%d", &novoRegistro.dados.idade);
    insere_registro(&novoRegistro);
  }
  printf("estrutura de hashing perfeito criada\n");
}

void insere_registro(Registro* registro) {
  FILE *arquivo_funcoes_hash;
  NivelUm nivelUm;
  
  arquivo_funcoes_hash = abre_arquivo(NOME_ARQUIVO_FUNCOES, "r+");
  fread(&nivelUm, sizeof(NivelUm), 1, arquivo_funcoes_hash);
  int posicaoNivelUm = hash(nivelUm.primo, nivelUm.tamanho, nivelUm.a, nivelUm.b, registro->dados.chave);

  NivelDois nivelDois;
  fseek(arquivo_funcoes_hash, sizeof(NivelUm) + sizeof(NivelDois) * posicaoNivelUm, SEEK_SET);
  fread(&nivelDois, sizeof(NivelDois), 1, arquivo_funcoes_hash);
  
  // printf("LOG - POSICAO NIVEL UM: %d - CHAVE: %d - A: %d - B: %d - PRIMO: %d\n", posicaoNivelUm, registro->dados.chave, nivelUm.a, nivelUm.b, nivelUm.primo);
  if (nivelDois.tamanho == 0) {
    nivelDois.tamanho = 1;
    fseek(arquivo_funcoes_hash, sizeof(NivelUm) + sizeof(NivelDois) * posicaoNivelUm, SEEK_SET);
    fwrite(&nivelDois, sizeof(NivelDois), 1, arquivo_funcoes_hash);
    fclose(arquivo_funcoes_hash);

    FILE *arquivo_registros;
    
    arquivo_registros = cria_arquivo_de_registros(get_nome_arquivo_registros(posicaoNivelUm), 1);
    fwrite(registro, sizeof(Registro), 1, arquivo_registros);
    fclose(arquivo_registros);
  } else {
    
    int tamanhoAntigo = nivelDois.tamanho;
    nivelDois.tamanho = pow(sqrt(nivelDois.tamanho) + 1, 2);

    //encontra a e b para função de hash que não dê conflito
    bool conflito = false;
    do {
      conflito = false;
      nivelDois.a = get_numero_aleatorio_entre(0, nivelUm.primo);
      nivelDois.b = get_numero_aleatorio_entre(1, nivelUm.primo);
      // printf("LOG - TENTANDO NOVA FUNCAO DE HASH - A: %d - B: %d\n", nivelDois.a, nivelDois.b);
      bool chavesAntigas[VALOR_MAXIMO_CHAVE] = {false};
      FILE* arquivo_registros = abre_arquivo(get_nome_arquivo_registros(posicaoNivelUm), "r");
      int posicaoNivelDois = hash(nivelUm.primo, nivelDois.tamanho, nivelDois.a, nivelDois.b, registro->dados.chave);
      chavesAntigas[posicaoNivelDois] = true;
      // printf("LOG - POSICAO NIVEL DOIS: %d\n", posicaoNivelDois);
      int i;
      for (i = 0; i < tamanhoAntigo; i++)
      {
        Registro registroAMover;
        fread(&registroAMover, sizeof(Registro), 1, arquivo_registros);
        if (registroAMover.ocupado)
        {
          int posicaoNivelDois = hash(nivelUm.primo, nivelDois.tamanho, nivelDois.a, nivelDois.b, registroAMover.dados.chave);
          // printf("LOG - POSICAO NIVEL DOIS: %d\n", posicaoNivelDois);
          if (chavesAntigas[posicaoNivelDois])
          {
            conflito = true;
            break;
          }
          chavesAntigas[posicaoNivelDois] = true;
        }
      }
      fclose(arquivo_registros);
      // fprintf(stderr, "LOG - CONFLITO: %d\n", conflito);
    } while (conflito);
    // printf("LOG - FUNCAO DE HASH ENCONTRADA - A: %d - B: %d\n", nivelDois.a, nivelDois.b);
    //mover de um arquivo para o outro
    FILE* arquivo_registros = abre_arquivo(get_nome_arquivo_registros(posicaoNivelUm), "r");
    FILE* arquivo_registros_novo = cria_arquivo_de_registros(get_nome_arquivo_registros_temp(posicaoNivelUm), nivelDois.tamanho);
    int posicaoNivelDois;
    posicaoNivelDois = hash(nivelUm.primo, nivelDois.tamanho, nivelDois.a, nivelDois.b, registro->dados.chave);
    fseek(arquivo_registros_novo, sizeof(Registro) * posicaoNivelDois, SEEK_SET);
    fwrite(registro, sizeof(Registro), 1, arquivo_registros_novo);
    int i;
    for (i = 0; i < tamanhoAntigo; i++)
    {
      Registro registroAMover;
      fread(&registroAMover, sizeof(Registro), 1, arquivo_registros);
      if (registroAMover.ocupado == true) {
        posicaoNivelDois = hash(nivelUm.primo, nivelDois.tamanho, nivelDois.a, nivelDois.b, registroAMover.dados.chave);
        fseek(arquivo_registros_novo, sizeof(Registro) * posicaoNivelDois, SEEK_SET);
        fwrite(&registroAMover, sizeof(Registro), 1, arquivo_registros_novo);
      }
    }
    fclose(arquivo_registros);
    fclose(arquivo_registros_novo);
    remove(get_nome_arquivo_registros(posicaoNivelUm));
    rename(get_nome_arquivo_registros_temp(posicaoNivelUm), get_nome_arquivo_registros(posicaoNivelUm));

    fseek(arquivo_funcoes_hash, sizeof(NivelUm) + sizeof(NivelDois) * posicaoNivelUm, SEEK_SET);
    fwrite(&nivelDois, sizeof(NivelDois), 1, arquivo_funcoes_hash);
    fclose(arquivo_funcoes_hash);
  }
}

void consulta_registro() {
  int chave;
  scanf("%d", &chave);
  NivelUm nivelUm;
  NivelDois nivelDois;

  FILE* arquivo_funcoes_hash = abre_arquivo(NOME_ARQUIVO_FUNCOES, "r");
  fread(&nivelUm, sizeof(NivelUm), 1, arquivo_funcoes_hash);
  int posicaoNivelUm = hash(nivelUm.primo, nivelUm.tamanho, nivelUm.a, nivelUm.b, chave);

  fseek(arquivo_funcoes_hash, sizeof(NivelUm) + sizeof(NivelDois) * posicaoNivelUm, SEEK_SET);
  fread(&nivelDois, sizeof(NivelDois), 1, arquivo_funcoes_hash);
  fclose(arquivo_funcoes_hash);
  
  if (nivelDois.tamanho == 0) {
    printf("chave nao encontrada: %d\n", chave);
    return;
  }

  int posicaoNivelDois = hash(nivelUm.primo, nivelDois.tamanho, nivelDois.a, nivelDois.b, chave);
  FILE* arquivo_registros = abre_arquivo(get_nome_arquivo_registros(posicaoNivelUm), "r");
  fseek(arquivo_registros, sizeof(Registro) * posicaoNivelDois, SEEK_SET);

  Registro registro;
  fread(&registro, sizeof(Registro), 1, arquivo_registros);

  if (registro.ocupado == true && registro.dados.chave == chave) {
    printf("chave: %d\n", registro.dados.chave);
    printf("%s\n", registro.dados.nome);
    printf("%d\n", registro.dados.idade);
  } else {
    printf("chave nao encontrada: %d\n", chave);
  }

  fclose(arquivo_registros);
}

void imprime_nivel_um() {
  FILE* arquivo_funcoes_hash = abre_arquivo(NOME_ARQUIVO_FUNCOES, "r");
  NivelUm nivelUm;
  fread(&nivelUm, sizeof(NivelUm), 1, arquivo_funcoes_hash);
  printf("hashing perfeito: primeiro nivel\n");
  printf("tamanho da tabela: %d\n", nivelUm.tamanho);
  printf("parametro a: %d\n", nivelUm.a);
  printf("parametro b: %d\n", nivelUm.b);
  printf("numero primo: %d\n", nivelUm.primo);
  for (int i = 0; i < nivelUm.tamanho; i++) {
    NivelDois nivelDois;
    fread(&nivelDois, sizeof(NivelDois), 1, arquivo_funcoes_hash);
    if (nivelDois.tamanho > 0)
    {
      FILE *arquivo_registros = abre_arquivo(get_nome_arquivo_registros(i), "r");
      Registro registro;

      printf("%d: ", i);

      for (int j = 0; j < nivelDois.tamanho; j++) {
        fread(&registro, sizeof(Registro), 1, arquivo_registros);
        if (registro.ocupado) {
          printf("%d ", registro.dados.chave);
        }
      }
      printf("\n");
      fclose(arquivo_registros);
    }
  }
  fclose(arquivo_funcoes_hash);
}

void imprime_nivel_dois() {
  FILE* arquivo_funcoes_hash = abre_arquivo(NOME_ARQUIVO_FUNCOES, "r");
  NivelUm nivelUm;
  fread(&nivelUm, sizeof(NivelUm), 1, arquivo_funcoes_hash);
  for (int i = 0; i < nivelUm.tamanho; i++) {
    NivelDois nivelDois;
    fread(&nivelDois, sizeof(NivelDois), 1, arquivo_funcoes_hash);
    if (nivelDois.tamanho > 0)
    {
      if (nivelDois.tamanho > 1) {
        FILE *arquivo_registros = abre_arquivo(get_nome_arquivo_registros(i), "r");
        Registro registro;
        printf("hashing perfeito: segundo nivel - indice: %d\n", i);
        printf("tamanho da tabela: %d\n", nivelDois.tamanho);
        printf("parametro a: %d\n", nivelDois.a);
        printf("parametro b: %d\n", nivelDois.b);
        printf("numero primo: %d\n", nivelUm.primo);
        for (int j = 0; j < nivelDois.tamanho; j++) {
          fread(&registro, sizeof(Registro), 1, arquivo_registros);
          if (registro.ocupado) {
            printf("%d: %d\n", j, registro.dados.chave);
          }
        }
        fclose(arquivo_registros);
      }
    }
  }
  fclose(arquivo_funcoes_hash);
}

void imprime_estrutura_global() {
  FILE* arquivo_funcoes_hash = abre_arquivo(NOME_ARQUIVO_FUNCOES, "r");
  NivelUm nivelUm;
  fread(&nivelUm, sizeof(NivelUm), 1, arquivo_funcoes_hash);
  printf("hashing perfeito: primeiro nivel\n");
  printf("tamanho da tabela: %d\n", nivelUm.tamanho);
  printf("parametro a: %d\n", nivelUm.a);
  printf("parametro b: %d\n", nivelUm.b);
  printf("numero primo: %d\n", nivelUm.primo);
  for (int i = 0; i < nivelUm.tamanho; i++) {
    NivelDois nivelDois;
    fread(&nivelDois, sizeof(NivelDois), 1, arquivo_funcoes_hash);
    if (nivelDois.tamanho > 0)
    {
      FILE *arquivo_registros = abre_arquivo(get_nome_arquivo_registros(i), "r");
      Registro registro;
      printf("hashing perfeito: segundo nivel - indice: %d\n", i);
      printf("tamanho da tabela: %d\n", nivelDois.tamanho);
      printf("parametro a: %d\n", nivelDois.a);
      printf("parametro b: %d\n", nivelDois.b);
      printf("numero primo: %d\n", nivelUm.primo);
      for (int j = 0; j < nivelDois.tamanho; j++) {
        fread(&registro, sizeof(Registro), 1, arquivo_registros);
        if (registro.ocupado) {
          printf("%d: %d\n", j, registro.dados.chave);
        }
      }
      fclose(arquivo_registros);
    }
  }
  fclose(arquivo_funcoes_hash);
}

void cardinalidade_funcoes_hashing() {
  FILE* arquivo_funcoes_hash = abre_arquivo(NOME_ARQUIVO_FUNCOES, "r");
  NivelUm nivelUm;
  fread(&nivelUm, sizeof(NivelUm), 1, arquivo_funcoes_hash);
  int cardinalidade = 1;
  for (int i = 0; i < nivelUm.tamanho; i++) {
    NivelDois nivelDois;
    fread(&nivelDois, sizeof(NivelDois), 1, arquivo_funcoes_hash);
    if (nivelDois.tamanho > 1)
    {
      cardinalidade++;
    }
  }
  printf("%d\n", cardinalidade);
}

FILE *abre_arquivo(char *nomeArquivo, char *modo){
  FILE *arquivo;
  if (!(arquivo = fopen(nomeArquivo, modo))) {
    printf("Erro na tentativa de abrir arquivo \"%s\".\n", nomeArquivo);
    exit(-1);
  }
  return arquivo;
}

FILE* cria_arquivo_de_registros(char* nome, int tamanho){
  FILE *arquivo;
  
  // if (arquivo = fopen(nome, "r")) {
  //   return arquivo;
  // }

  arquivo = abre_arquivo(nome, "w+");
  int i;
  Registro registro;
  registro.ocupado = false;
  for (i = 0; i < tamanho; i++) {
    fwrite(&registro, sizeof(Registro), 1, arquivo);
  }
  fseek(arquivo, 0, SEEK_SET);
  return arquivo;
}

int proximo_primo(int n){
  int primos[26] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101};
  int i;
  for (i = 0; i < 26; i++) {
    if (primos[i] > n) {
      return primos[i];
    }
  }
  printf("Erro: não há mais primos.\n");
  exit(-1);
}

// retorna um número aleatório n, min <= n < max
int get_numero_aleatorio_entre(int min, int max) {
  return (rand() % (max - min)) + min;
}

int hash(int p, int m, int a, int b, int k) {
  return ((a * k + b) % p) % m;
}

char* get_nome_arquivo_registros(int posicao) {
  char* nomeArquivo = malloc(sizeof(char) * 100);
  sprintf(nomeArquivo, "data_registros_%d.bin", posicao);
  return nomeArquivo;
}

char* get_nome_arquivo_registros_temp(int posicao) {
  char* nomeArquivo = malloc(sizeof(char) * 100);
  sprintf(nomeArquivo, "data_registros_%d_tmp.bin", posicao);
  return nomeArquivo;
}