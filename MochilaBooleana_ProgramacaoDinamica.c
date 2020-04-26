/* --------------------------------------------------------------
                PUC Minas - Pocos de Caldas
                     1o semestre 2020
                    Programacao Pararela

            Aluno: Leonardo Oliveira de Moura
            Aluno: Matheus Inhesta

Resolve o problema da mochila booleana de forma iterativa usando 
programacao dinamica.
Nao utiliza processamento paralelo.
Usado para comaparar o tempo de execucao de um codigo por forca bruta com
processamento paralelo com um executado em uma maquina com programacao
dinamica.

--------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#define ERRO -1
#define FALSE 0
#define TRUE 1
#define TAM_LINHA 10

typedef struct
{
    int id;
    int peso;
    int valor;
} objeto;

int EncontraValorMaximo(int, int, objeto*);
int LeArquivo(int*, int*, objeto**);
int InicializaMemoria(int, int);
int LeValorDaMemoria(int indiceObjeto, int capacidade);
void EscreveValorNaMemoria(int indiceObjeto, int capacidade, int valor);
int AbreArquivo(FILE **);
void printaCabecalhoErro();
void printaObjetos(int, objeto*);
int LeInteiro(FILE*);
objeto* LeObjetos(int, FILE*);
objeto TransformarLinhaEmObjeto(char*, int);
char **SplitString(char*);

int **memoria;

int main()
{
    int loop, valorMaximo;
    int capacidade, qtdeObjetos;
    clock_t inicio, fim;
    objeto *objetos;

    if (!LeArquivo(&capacidade, &qtdeObjetos, &objetos))
        return 1;

    if (!InicializaMemoria(capacidade, qtdeObjetos))
        return 1;

    printf("\n\nRealizando a busca de combinacoes.");
    inicio = clock();
    valorMaximo = EncontraValorMaximo(capacidade, qtdeObjetos, objetos);
    fim = clock();

    printf("\nValor maximo suportado: %d", valorMaximo);
    printf("\nTempo gasto em milissegundos: %g", ((double)(fim - inicio)) * 1000.0 / CLOCKS_PER_SEC);
    printf("\n\n");
}

int EncontraValorMaximo(int capacidade, int qtdeObjetos, objeto *objetos)
{
    int loop, valorMaximo, valorDoLoop, capacidadeDoLoop;

    if (capacidade == 0) return 0;

    valorMaximo = 0;
    valorDoLoop = 0;
    capacidadeDoLoop = 0;
    for (loop = 0; loop < qtdeObjetos; loop++)
    {
        if (LeValorDaMemoria(loop, capacidade) == -1)
        {
            if (objetos[loop].peso > capacidade)
            {
                EscreveValorNaMemoria(loop, capacidade, INT_MIN);
                continue;
            }

            valorDoLoop = objetos[loop].valor;
            capacidadeDoLoop = capacidade - objetos[loop].peso;

            valorDoLoop += EncontraValorMaximo(capacidadeDoLoop, qtdeObjetos, objetos);
            EscreveValorNaMemoria(loop, capacidade, valorDoLoop);
        }
        else
        {
            valorDoLoop = LeValorDaMemoria(loop, capacidade);
        }
        

        if (valorDoLoop > valorMaximo)
            valorMaximo = valorDoLoop;
    }

    return valorMaximo;
}

int LeArquivo(int *capacidade, int *qtdeObjetos, objeto **objetos)
{
    int loop;
    FILE *dados;

    if (!AbreArquivo(&dados))
        return FALSE;

    if ((*capacidade = LeInteiro(dados)) == ERRO)
        return FALSE;
    printf("\nCapacidade: %d", *capacidade);

    if ((*qtdeObjetos = LeInteiro(dados)) == ERRO)
        return FALSE;
    printf("\nQuantidade de objetos: %d", *qtdeObjetos);
    
    if ((*objetos = LeObjetos(*qtdeObjetos, dados)) == NULL)
        return FALSE;

    fclose(dados);

    return TRUE;
}

int InicializaMemoria(int capacidade, int qtdeObjetos)
{
    int loopLinhas, loopColunas;
    int tamanhoMatriz = (sizeof(int) * capacidade) * (sizeof(int) * qtdeObjetos);
    memoria = (int **) malloc(sizeof(int) * qtdeObjetos);
    int cont = 0;

    if (memoria == NULL)
    {
        printaCabecalhoErro();
        printf("Não foi possível resevar a memória necessária.");
        return FALSE;
    }

    for (loopLinhas = 0; loopLinhas < qtdeObjetos; loopLinhas++)
    {
        memoria[loopLinhas] = (int *) malloc(sizeof(int) * capacidade);
        for (loopColunas = 0; loopColunas < capacidade; loopColunas++)
        {
            memoria[loopLinhas][loopColunas] = -1;
        }
    }

    return TRUE;
}

int LeValorDaMemoria(int indiceObjeto, int capacidade)
{
    return memoria[indiceObjeto][capacidade - 1];
}

void EscreveValorNaMemoria(int indiceObjeto, int capacidade, int valor)
{
    //printf("\nEscrevendo objeto %d, capacidade %d = %d", indiceObjeto, capacidade - 1, valor);
    memoria[indiceObjeto][capacidade - 1] = valor;
}

int AbreArquivo(FILE **dados)
{
    FILE *arquivo;
    if ((arquivo = fopen("dados.txt", "r")) == NULL)
    {
        printaCabecalhoErro();
        printf("Nao foi possivel abrir o arquivo 'dados.txt'.");
        return FALSE;
    }

    if(feof(arquivo))
    {
        printaCabecalhoErro();
        printf("Arquivo 'dados.txt' vazio.");
        return FALSE;
    }

    *dados = arquivo;
    return TRUE;
}

void printaCabecalhoErro()
{
    printf("\n\n---------- ERRO ----------\n");
}

void printaObjetos(int qtdeObjetos, objeto *objetos)
{
    int loop;
    for (loop = 0; loop < qtdeObjetos; loop++)
    {
        printf("\nid: %d", objetos[loop].id);
        printf("\nPeso: %d", objetos[loop].peso);
        printf("\nValor: %d", objetos[loop].valor);
        printf("\n");
    }
}

int LeInteiro(FILE *dados)
{
    char linha[TAM_LINHA];

    memset(linha, 0, sizeof(char) * TAM_LINHA);
    if (fgets(linha, TAM_LINHA, dados) == NULL)
    {
        printaCabecalhoErro();
        printf("Erro ao carregar valor do arquivo.");
        return ERRO;
    }
    return atoi(linha);
}

objeto* LeObjetos(int qtdeObjetos, FILE *dados)
{
    int loop;
    char linha[TAM_LINHA];
    objeto *objetos;

    objetos = (objeto*) malloc(sizeof(objeto) * qtdeObjetos);
    loop = 0;
    while(fgets(linha, TAM_LINHA, dados) != NULL)
    {
        if (loop >= qtdeObjetos)
        {
            printaCabecalhoErro();
            printf("Existem mais objetos do que o informado no totalizador.");
            free(objetos);
            return NULL;
        }

        objetos[loop] = TransformarLinhaEmObjeto(linha, loop);

        if (objetos[loop].peso == 0 || objetos[loop].valor == 0)
        {
            printaCabecalhoErro();
            printf("Erro ao ler objeto na linha %d", loop + 3);
            free(objetos);
            return NULL;
        }

        loop++;
    }

    if (loop < qtdeObjetos)
        printf("\n\nATENCAO: Existem menos objetos do que o informado no totalizador.");

    return objetos;
}

objeto TransformarLinhaEmObjeto(char linha[], int id)
{
    objeto retorno;

    char **informacoesSeparadas = SplitString(linha);

    retorno.id = id;
    retorno.peso = atoi(informacoesSeparadas[0]);
    retorno.valor = atoi(informacoesSeparadas[1]);

    free(informacoesSeparadas[0]);
    free(informacoesSeparadas[1]);
    free(informacoesSeparadas);

    return retorno;
}

char **SplitString(char stringInteira[])
{
    char **retorno;
    int loop, posicaoEspaco;
    retorno = malloc(sizeof(char*) * 2);
    retorno[0] = malloc(sizeof(char) * TAM_LINHA);
    retorno[1] = malloc(sizeof(char) * TAM_LINHA);

    for(loop = 0; loop < TAM_LINHA; loop++)
    {
        if (stringInteira[loop] == ' ')
        {
            posicaoEspaco = loop;
            break;
        }
    }

    for (loop = 0; loop < posicaoEspaco; loop++)
        retorno[0][loop] = stringInteira[loop];
    retorno [0][loop] = '\0';

    for (loop = 0; loop < strlen(stringInteira) - (posicaoEspaco + 1); loop++)
        retorno[1][loop] = stringInteira[loop + posicaoEspaco + 1];
    retorno[1][loop] = '\0';

    return retorno;
}
