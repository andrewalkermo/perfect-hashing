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
  // seed para gerar numeros aleatorios
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

// cria arquivo de funções de hashing, com N+1 funções
// onde N é o tamanho do arquivo de registros
// o indice 0 é a função de hashing padrãode nivel 1
// os indices 1 a N são as funções de hashing de nivel 2
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
  
  // cria o arquivo de registros, se for o primeiro registro, e lê o registro, ou se não, lê o o arquivo existente
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

      // lê o registro na posição a ser movida, no arquivo de registros novo
      Registro registroJaMovido;
      fseek(arquivo_registros_novo, sizeof(Registro) * posicaoNivelDois, SEEK_SET);
      fread(&registroJaMovido, sizeof(Registro), 1, arquivo_registros_novo);

      // se a posição estiver vazia, salva o registro, se não, tenta inserir novamente, recursivamente
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

  // lê a função de hashing e pega a posição do nivel um
  FILE* arquivo_funcoes_hash = abre_arquivo(NOME_ARQUIVO_FUNCOES, "r");
  fread(&nivelUm, sizeof(NivelUm), 1, arquivo_funcoes_hash);
  int posicaoNivelUm = hash(nivelUm.primo, nivelUm.tamanho, nivelUm.a, nivelUm.b, chave);

  // lê a função de hashing e pega a posição do nivel dois
  fseek(arquivo_funcoes_hash, sizeof(NivelUm) + sizeof(NivelDois) * posicaoNivelUm, SEEK_SET);
  fread(&nivelDois, sizeof(NivelDois), 1, arquivo_funcoes_hash);
  fclose(arquivo_funcoes_hash);
  
  // se o tamanho do nivel dois for 0, não existe registro com a chave
  if (nivelDois.tamanho == 0) {
    printf("chave nao encontrada: %d\n", chave);
    return;
  }

  // abre o arquivo de registros
  int posicaoNivelDois = hash(nivelUm.primo, nivelDois.tamanho, nivelDois.a, nivelDois.b, chave);
  FILE* arquivo_registros = abre_arquivo(get_nome_arquivo_registros(posicaoNivelUm), "r");
  fseek(arquivo_registros, sizeof(Registro) * posicaoNivelDois, SEEK_SET);

  // lê o registro
  Registro registro;
  fread(&registro, sizeof(Registro), 1, arquivo_registros);

  // printa o registro, caso exista
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

  //impreme o nivel um
  printf("hashing perfeito: primeiro nivel\n");
  printf("tamanho da tabela: %d\n", nivelUm.tamanho);
  printf("parametro a: %d\n", nivelUm.a);
  printf("parametro b: %d\n", nivelUm.b);
  printf("numero primo: %d\n", nivelUm.primo);

  // loop para imprimir as chances existentes no nivel dois de cada indice
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

  int indice;
  scanf("%d", &indice);
  
  NivelDois nivelDois;

  fseek(arquivo_funcoes_hash, sizeof(NivelUm) + sizeof(NivelDois) * indice, SEEK_SET);
  fread(&nivelDois, sizeof(NivelDois), 1, arquivo_funcoes_hash);
  
  if (nivelDois.tamanho > 0) {
    FILE *arquivo_registros = abre_arquivo(get_nome_arquivo_registros(indice), "r");
    Registro registro;

    // imprime o nivel dois
    printf("hashing perfeito: segundo nivel - indice: %d\n", indice);
    printf("tamanho da tabela: %d\n", nivelDois.tamanho);
    printf("parametro a: %d\n", nivelDois.a);
    printf("parametro b: %d\n", nivelDois.b);
    printf("numero primo: %d\n", nivelUm.primo);

    // loop para imprimir as chaves existentes no nivel dois de cada indice
    for (int j = 0; j < nivelDois.tamanho; j++) {
      fread(&registro, sizeof(Registro), 1, arquivo_registros);
      if (registro.ocupado) {
        printf("%d: %d\n", j, registro.dados.chave);
      }
    }

    fclose(arquivo_registros);
  }
  fclose(arquivo_funcoes_hash);
}

void imprime_estrutura_global() {
  FILE* arquivo_funcoes_hash = abre_arquivo(NOME_ARQUIVO_FUNCOES, "r");
  NivelUm nivelUm;
  fread(&nivelUm, sizeof(NivelUm), 1, arquivo_funcoes_hash);

  // imprime o nivel um
  printf("hashing perfeito: primeiro nivel\n");
  printf("tamanho da tabela: %d\n", nivelUm.tamanho);
  printf("parametro a: %d\n", nivelUm.a);
  printf("parametro b: %d\n", nivelUm.b);
  printf("numero primo: %d\n", nivelUm.primo);

  // loop para imprimir as informacoes do nitel dois de cada indice
  for (int i = 0; i < nivelUm.tamanho; i++) {
    NivelDois nivelDois;
    fread(&nivelDois, sizeof(NivelDois), 1, arquivo_funcoes_hash);
    if (nivelDois.tamanho > 0)
    {
      FILE *arquivo_registros = abre_arquivo(get_nome_arquivo_registros(i), "r");
      Registro registro;

      // imprime o nivel dois
      printf("hashing perfeito: segundo nivel - indice: %d\n", i);
      printf("tamanho da tabela: %d\n", nivelDois.tamanho);
      printf("parametro a: %d\n", nivelDois.a);
      printf("parametro b: %d\n", nivelDois.b);
      printf("numero primo: %d\n", nivelUm.primo);

      // loop para imprimir as chaves de cada indice do nivel dois
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

// imprime a cardinalidade de do conjuntos de funções de hashing = primo * (primo - 1)
void cardinalidade_funcoes_hashing() {
  FILE* arquivo_funcoes_hash = abre_arquivo(NOME_ARQUIVO_FUNCOES, "r");
  NivelUm nivelUm;
  fread(&nivelUm, sizeof(NivelUm), 1, arquivo_funcoes_hash);
  int cardinalidade = nivelUm.primo * nivelUm.primo - 1;
  printf("%d\n", cardinalidade);
}

// retorna um arquivo, ou da erro se não conseguir abrir, com as permições informadas
FILE *abre_arquivo(char *nomeArquivo, char *modo){
  FILE *arquivo;
  if (!(arquivo = fopen(nomeArquivo, modo))) {
    printf("Erro na tentativa de abrir arquivo \"%s\".\n", nomeArquivo);
    exit(-1);
  }
  return arquivo;
}

// cria arquivo de registro em branco
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

// retorna o valor do proximo primo maior que o valor informado
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

// função de hashing
int hash(int p, int m, int a, int b, int k) {
  return ((a * k + b) % p) % m;
}

// retorna o nome do arquivo de registros, de acordo com o indice
char* get_nome_arquivo_registros(int posicao) {
  char* nomeArquivo = malloc(sizeof(char) * 100);
  sprintf(nomeArquivo, "data_registros_%d.dat", posicao);
  return nomeArquivo;
}


// retorna o nome do arquivo de registros temporario, de acordo com o indice
char* get_nome_arquivo_registros_temp(int posicao) {
  char* nomeArquivo = malloc(sizeof(char) * 100);
  sprintf(nomeArquivo, "data_registros_%d_tmp.dat", posicao);
  return nomeArquivo;
}