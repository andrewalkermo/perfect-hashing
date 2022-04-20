#include <stdio.h>
#include <stdlib.h>

#define TAMANHO_NOME 20
#define MAXNUMREGS 11

#define INSERE_REGISTRO 'i'
#define CONSULTA_REGISTRO 'c'
#define REMOVE_REGISTRO 'r'
#define IMPRIME_ARQUIVO 'p'
#define MEDIA_DE_ACESSOS 'm'
#define FIM_DOS_COMANDOS 'e'

#define FILE_NAME "registros"

#define STATUS_OCUPADO 1
#define STATUS_LIVRE 0
#define STATUS_REMOVIDO -1

int totalAcessosComSucesso = 0;
int totalAcessosComFalha = 0;
int totalConsultasComSucesso = 0;
int totalConsultasComFalha = 0;

typedef struct {
  int chave;
  char nome[TAMANHO_NOME];
  int idade;
} DadosUsuario;

typedef struct {
  int status;
  DadosUsuario dados;
} Registro;

int hashUm(int chave);
int hashDois(int chave);
int max (int a, int b);
int proximaPosicao(int posicaoAtual, int salto);

void lerComandos();
void inserirRegistro();
void consultarRegistro();
void removerRegistro();
void imprimirArquivo();
void mediaDeAcessos();
void inicializarArquivo();
void registraSucesso(int acessos);
void registraFalha(int acessos);

FILE *abreArquivo(char *nomeArquivo, char *modo);

int main() {
  inicializarArquivo();
  lerComandos();
  return 0;
}

void lerComandos(){
  char comando;
  while (scanf("%s", &comando)) {
    switch (comando) {
      case INSERE_REGISTRO:
        inserirRegistro();
        break;
      case CONSULTA_REGISTRO:
        consultarRegistro();
        break;
      case REMOVE_REGISTRO:
        removerRegistro();
        break;
      case IMPRIME_ARQUIVO:
        imprimirArquivo();
        break;
      case MEDIA_DE_ACESSOS:
        mediaDeAcessos();
        break;
      case FIM_DOS_COMANDOS:
        return;
      default:
        exit(-1);
    }
  }
}

void inserirRegistro() {
  Registro novoRegistro;
  novoRegistro.status = STATUS_OCUPADO;
  scanf("%d", &novoRegistro.dados.chave);
  scanf("%s", novoRegistro.dados.nome);
  scanf("%d", &novoRegistro.dados.idade);

  Registro registro;
  FILE *arquivo = abreArquivo(FILE_NAME, "r+");
  int posicao = hashUm(novoRegistro.dados.chave);
  int acessos = 0;
  
  do {
    acessos++;
    fseek(arquivo, posicao * sizeof(Registro), SEEK_SET);
    fread(&registro, sizeof(Registro), 1, arquivo);

    if (registro.status != STATUS_OCUPADO) {
      fseek(arquivo, posicao *sizeof(Registro), SEEK_SET);
      fwrite(&novoRegistro, sizeof(Registro), 1, arquivo);
      printf("insercao com sucesso: %d\n", novoRegistro.dados.chave);
      fclose(arquivo);
      registraSucesso(acessos);
      return;
    } else if (registro.dados.chave == novoRegistro.dados.chave) {
      printf("chave ja existente: %d\n", novoRegistro.dados.chave);
      fclose(arquivo);
      return;
      registraFalha(acessos);
    }
    
    posicao = proximaPosicao(posicao, hashDois(novoRegistro.dados.chave));
  } while (acessos < MAXNUMREGS);

  registraFalha(acessos);
  printf("insercao de chave sem sucesso - arquivo cheio: %d\n", novoRegistro.dados.chave);
  fclose(arquivo);
}

void consultarRegistro() {
  int chave;
  scanf("%d", &chave);
  Registro registro;
  FILE *arquivo = abreArquivo(FILE_NAME, "r");
  
  int posicao = hashUm(chave);
  int acessos = 0;
  do {
    acessos++;
    fseek(arquivo, posicao * sizeof(Registro), SEEK_SET);
    fread(&registro, sizeof(Registro), 1, arquivo);
    
    if (registro.dados.chave == chave && registro.status == STATUS_OCUPADO) {
      printf("chave: %d\n", chave);
      printf("%s\n", registro.dados.nome);
      printf("%d\n", registro.dados.idade);
      fclose(arquivo);
      registraSucesso(acessos);
      return;
    }
    posicao = proximaPosicao(posicao, hashDois(chave));
  } while (acessos < MAXNUMREGS && registro.status != STATUS_LIVRE);

  totalAcessosComFalha += acessos;
  totalConsultasComFalha++;
  printf("chave nao encontrada: %d\n", chave);
  fclose(arquivo);
}

void removerRegistro(){
  int chave;
  scanf("%d", &chave);
  Registro registro;
  FILE *arquivo = abreArquivo(FILE_NAME, "r+");
  
  int posicao = hashUm(chave);
  int acessos = 0;
  do {
    acessos++;
    fseek(arquivo, posicao * sizeof(Registro), SEEK_SET);
    fread(&registro, sizeof(Registro), 1, arquivo);
    
    if (registro.dados.chave == chave && registro.status != STATUS_REMOVIDO) {
      registro.status = STATUS_REMOVIDO;
      fseek(arquivo, posicao * sizeof(Registro), SEEK_SET);
      fwrite(&registro, sizeof(Registro), 1, arquivo);
      printf("chave removida com sucesso: %d\n", chave);
      fclose(arquivo);
      registraSucesso(acessos);
      return;
    }

    posicao = proximaPosicao(posicao, hashDois(chave));
  } while (acessos < MAXNUMREGS && registro.status != STATUS_LIVRE);

  printf("chave nao encontrada: %d\n", chave);
  fclose(arquivo);
  registraFalha(acessos);
}

void imprimirArquivo(){
  int i;
  Registro registro;
  FILE *arquivo = abreArquivo(FILE_NAME, "r");
  
  for (i = 0; i < MAXNUMREGS; i++) {
    fread (&registro, sizeof (Registro), 1, arquivo);
    if (registro.status == STATUS_OCUPADO) {
      printf("%d: %d %s %d\n", i, registro.dados.chave, registro.dados.nome, registro.dados.idade);
    } else if (registro.status == STATUS_LIVRE) {
      printf("%d: vazio\n", i);
    } else {
      printf("%d: *\n", i);
    }
  }
  
  fclose(arquivo);
}

void mediaDeAcessos(){
  printf("%0.1f\n", (float) totalAcessosComSucesso / totalConsultasComSucesso);
  printf("%0.1f\n", (float) totalAcessosComFalha / totalConsultasComFalha);
}

void inicializarArquivo(){
  FILE *arquivo;
  if (arquivo = fopen(FILE_NAME, "r")) {
    fclose(arquivo);
    return;
  }

  int i;
  arquivo = abreArquivo(FILE_NAME, "w+");
  Registro registro;
  registro.status = STATUS_LIVRE;
  
  for (i = 0; i < MAXNUMREGS; i++) {
    fwrite(&registro, sizeof(Registro), 1, arquivo);
  }
  fclose(arquivo);
}

int hashUm(int chave){
  return chave % MAXNUMREGS;
}

int hashDois(int chave){
  return max((chave / MAXNUMREGS) % MAXNUMREGS, 1);
}

int max (int a, int b){
  return a > b ? a : b;
}

FILE *abreArquivo(char *nomeArquivo, char *modo){
  FILE *arquivo;
  if (!(arquivo = fopen(nomeArquivo, modo))) {
    printf("Erro na tentativa de abrir arquivo \"%s\".\n", nomeArquivo);
    exit(-1);
  }
  return arquivo;
}

int proximaPosicao(int posicaoAtual, int salto){
  return (posicaoAtual + salto) % MAXNUMREGS;
}

void registraSucesso(int acessos){
  totalAcessosComSucesso += acessos;
  totalConsultasComSucesso++;
}

void registraFalha(int acessos){
  totalAcessosComFalha += acessos;
  totalConsultasComFalha++;
}