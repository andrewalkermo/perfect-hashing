#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tiposArquivo.h"

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

  arquivo = abre_arquivo(NOME_ARQUIVO_FUNCOES, "w+");

  NivelUm nivelUm;
  nivelUm.tamanho = tamanho;
  nivelUm.primo = primo;
  nivelUm.a = TEST_A != -1 ? TEST_A : get_numero_aleatorio_entre(0, primo);
  nivelUm.b = TEST_B != -1 ? TEST_B : get_numero_aleatorio_entre(1, primo);

  fwrite(&nivelUm, sizeof(NivelUm), 1, arquivo);

  NivelDois nivelDois;
  nivelDois.tamanho = 0;
  nivelDois.a = get_numero_aleatorio_entre(0, primo);
  nivelDois.b = get_numero_aleatorio_entre(1, primo);
  
  for (int i = 0; i < tamanho; i++) {
    fwrite(&nivelDois, sizeof(NivelDois), 1, arquivo);
  }
  fclose(arquivo);
}

void insere_registros() {
  int tamanho;
  scanf("%d", &tamanho);

  // loop que insere os registros
  for (int i = 0; i < tamanho; i++) {
    Registro novoRegistro;
    novoRegistro.ocupado = true;
    scanf("%d", &novoRegistro.dados.chave);
    getchar();

    // se for o primeiro registro, cria o arquivo de funções
    if (i == 0) {
      int primo = proximo_primo(novoRegistro.dados.chave);
      cria_arquivo_de_funcoes(tamanho, primo);
    }
    // lê a linha inteira até o \n
    fgets(novoRegistro.dados.nome, TAMANHO_NOME, stdin);
    
    scanf("%d", &novoRegistro.dados.idade);
    insere_registro(&novoRegistro);
  }
  printf("estrutura de hashing perfeito criada\n");
}

void insere_registro(Registro* registro) {
  FILE *arquivo_funcoes_hash;
  NivelUm nivelUm;
  
  // abre o arquivo de funções e le a função de hashing de nivel um
  arquivo_funcoes_hash = abre_arquivo(NOME_ARQUIVO_FUNCOES, "r+");
  fread(&nivelUm, sizeof(NivelUm), 1, arquivo_funcoes_hash);
  int posicaoNivelUm = hash(nivelUm.primo, nivelUm.tamanho, nivelUm.a, nivelUm.b, registro->dados.chave);

  // lê a função de hashing de nivel dois
  NivelDois nivelDois;
  fseek(arquivo_funcoes_hash, sizeof(NivelUm) + sizeof(NivelDois) * posicaoNivelUm, SEEK_SET);
  fread(&nivelDois, sizeof(NivelDois), 1, arquivo_funcoes_hash);
  fclose(arquivo_funcoes_hash);
  
  // salva o tamanho antigo do nivel dois e incrementa o tamanho do nivel dois
  int tamanhoAntigo = nivelDois.tamanho;
  nivelDois.tamanho = pow(sqrt(nivelDois.tamanho) + 1, 2);

  // encontra_a_b(&nivelUm, &nivelDois, registro, posicaoNivelUm, tamanhoAntigo);
  nivelDois.a = get_numero_aleatorio_entre(0, nivelUm.primo);
  nivelDois.b = get_numero_aleatorio_entre(1, nivelUm.primo);
  
  FILE* arquivo_registros;
  if (tamanhoAntigo == 0) {
    arquivo_registros = cria_arquivo_de_registros(get_nome_arquivo_registros(posicaoNivelUm), nivelDois.tamanho);
  }
  else {
    arquivo_registros = abre_arquivo(get_nome_arquivo_registros(posicaoNivelUm), "r+");
  }

  FILE* arquivo_registros_novo = cria_arquivo_de_registros(get_nome_arquivo_registros_temp(posicaoNivelUm), nivelDois.tamanho);
  int posicaoNivelDois;

  // salva novo registro no novo arquivo de registros
  posicaoNivelDois = hash(nivelUm.primo, nivelDois.tamanho, nivelDois.a, nivelDois.b, registro->dados.chave);
  fseek(arquivo_registros_novo, sizeof(Registro) * posicaoNivelDois, SEEK_SET);
  fwrite(registro, sizeof(Registro), 1, arquivo_registros_novo);

  // loop que salva os registros no novo arquivo de registros
  for (int i = 0; i < tamanhoAntigo; i++)
  {
    Registro registroAMover;
    fread(&registroAMover, sizeof(Registro), 1, arquivo_registros);
    if (registroAMover.ocupado == true) {
      posicaoNivelDois = hash(nivelUm.primo, nivelDois.tamanho, nivelDois.a, nivelDois.b, registroAMover.dados.chave);

      Registro registroJaMovido;
      fseek(arquivo_registros_novo, sizeof(Registro) * posicaoNivelDois, SEEK_SET);
      fread(&registroJaMovido, sizeof(Registro), 1, arquivo_registros_novo);

      if (registroJaMovido.ocupado == false) {
        fseek(arquivo_registros_novo, sizeof(Registro) * posicaoNivelDois, SEEK_SET);
        fwrite(&registroAMover, sizeof(Registro), 1, arquivo_registros_novo);
      } else {
        fclose(arquivo_registros);
        fclose(arquivo_registros_novo);
        remove(get_nome_arquivo_registros_temp(posicaoNivelUm));
        insere_registro(registro);
        return;
      }
    }
  }

  fclose(arquivo_registros);
  fclose(arquivo_registros_novo);

  // remove o arquivo de registros antigo e substitui pelo novo
  remove(get_nome_arquivo_registros(posicaoNivelUm));
  rename(get_nome_arquivo_registros_temp(posicaoNivelUm), get_nome_arquivo_registros(posicaoNivelUm));

  // salva a função de hashing de nivel dois com novos valores de a, b e tamanho
  arquivo_funcoes_hash = abre_arquivo(NOME_ARQUIVO_FUNCOES, "r+");
  fseek(arquivo_funcoes_hash, sizeof(NivelUm) + sizeof(NivelDois) * posicaoNivelUm, SEEK_SET);
  fwrite(&nivelDois, sizeof(NivelDois), 1, arquivo_funcoes_hash);
  fclose(arquivo_funcoes_hash);
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
    printf("%s", registro.dados.nome);
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
  int cardinalidade = nivelUm.primo * nivelUm.primo - 1;
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
  
  arquivo = abre_arquivo(nome, "w+");

  Registro registro;
  registro.ocupado = false;
  for (int i = 0; i < tamanho; i++) {
    fwrite(&registro, sizeof(Registro), 1, arquivo);
  }
  fseek(arquivo, 0, SEEK_SET);
  return arquivo;
}

int proximo_primo(int n){
  int primos[26] = {2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101};

  for (int i = 0; i < 26; i++) {
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
  sprintf(nomeArquivo, "data_registros_%d.dat", posicao);
  return nomeArquivo;
}

char* get_nome_arquivo_registros_temp(int posicao) {
  char* nomeArquivo = malloc(sizeof(char) * 100);
  sprintf(nomeArquivo, "data_registros_%d_tmp.dat", posicao);
  return nomeArquivo;
}