/* --------------------------------------------------------------
                PUC Minas - Pocos de Caldas
                     1o semestre 2020
                    Programacao Pararela

            Aluno: Leonardo Oliveira de Moura
            Aluno: Matheus Inhesta

Resolve o problema da mochila booleana de forma iterativa usando forca bruta.
Nao utiliza programacao dinamica.
Utiliza a abordagem de Job Farm para distribuição das tarefas entre
as unidades de processamento.

--------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "mpi.h"

#define ERRO -1
#define FALSE 0
#define TRUE 1
#define TAM_LINHA 10
#define TAG 2587

typedef struct
{
    int id;
    int peso;
    int valor;
} objeto;

int EncontraValorMaximoDeFormaDistribuida(int capacidade, int qtdeObjetos, objeto *objetos);
int SincronizaValorMaximoEntreUnidadesDeProcessamento(int valorMaximoAtual);
int RecebeValoresMaximosDeMaquinasSlave();
int EncontraValorMaximo(int, int, objeto*);
int LeArquivo(int*, int*, objeto**);
int AbreArquivo(FILE **);
void PrintaInformacoesDoArquivo(int capacidade, int qtdeObjetos);
void PrintaCabecalhoErro();
void PrintaObjetos(int, objeto*);
int LeInteiro(FILE*);
objeto* LeObjetos(int, FILE*);
objeto TransformarLinhaEmObjeto(char*, int);
char **SplitString(char*);

//Funcoes de acesso a informacoes da biblioteca MPI
int GetIdDaUnidadeDeProcessamento(); //rank
int GetNumeroDeUnidadesDeProcessamento(); //size

int main(int argc, char *argv[])
{
    int valorMaximo;
    int capacidade, qtdeObjetos;
    clock_t inicio, fim;
    objeto *objetos;

    MPI_Init(&argc, &argv);

    if (!LeArquivo(&capacidade, &qtdeObjetos, &objetos))
        return 1;

    PrintaInformacoesDoArquivo(capacidade, qtdeObjetos);
    
    inicio = clock();
    valorMaximo = EncontraValorMaximoDeFormaDistribuida(capacidade, qtdeObjetos, objetos);
    fim = clock();

    valorMaximo = SincronizaValorMaximoEntreUnidadesDeProcessamento(valorMaximo);
	if(GetIdDaUnidadeDeProcessamento() == 0){
	    printf("\nValor maximo suportado: %d", valorMaximo);
	    printf("\nTempo gasto em milissegundos: %g", ((double)(fim - inicio)) * 1000.0 / CLOCKS_PER_SEC);
	    printf("\n\n");
	}

	MPI_Finalize();
}

int EncontraValorMaximoDeFormaDistribuida(int capacidade, int qtdeObjetos, objeto *objetos)
{
    int loop, valorMaximo, valorDoLoop, capacidadeDoLoop;

    valorMaximo = 0;
    valorDoLoop = 0;
    capacidadeDoLoop = 0;

    for(loop = GetIdDaUnidadeDeProcessamento(); loop < qtdeObjetos; loop += getNumeroDeUnidadesDeProcessamento())
    {
        if (objetos[loop].peso > capacidade)
            continue;

        valorDoLoop = objetos[loop].valor;
        capacidadeDoLoop = capacidade - objetos[loop].peso;

        valorDoLoop += EncontraValorMaximo(capacidadeDoLoop, qtdeObjetos, objetos);

        if (valorDoLoop > valorMaximo)
            valorMaximo = valorDoLoop;
    }

    return valorMaximo;
}

int SincronizaValorMaximoEntreUnidadesDeProcessamento(int valorMaximoAtual)
{
    int valorMaximoDeOutrasMaquinas = 0;
    if (GetIdDaUnidadeDeProcessamento() == 0)
    {
        valorMaximoDeOutrasMaquinas = RecebeValoresMaximosDeMaquinasSlave();
    }
    else
    {
        MPI_Send(&valorMaximoAtual, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD);
    }
    
    if (valorMaximoDeOutrasMaquinas > valorMaximoAtual)
        valorMaximoAtual = valorMaximoDeOutrasMaquinas;

    return valorMaximoAtual;
}

int RecebeValoresMaximosDeMaquinasSlave()
{
    int valorMaximo = 0, valorDoLoop = 0, loop;

    for (loop = 1; loop < getNumeroDeUnidadesDeProcessamento(); loop++)
    {
        MPI_Recv(&valorDoLoop, 1, MPI_INT, loop, TAG, MPI_COMM_WORLD, NULL);

        if (valorDoLoop > valorMaximo)
            valorMaximo = valorDoLoop;
    }

    return valorMaximo;
}

int EncontraValorMaximo(int capacidade, int qtdeObjetos, objeto *objetos)
{
    int loop, valorMaximo, valorDoLoop, capacidadeDoLoop;

    valorMaximo = 0;
    valorDoLoop = 0;
    capacidadeDoLoop = 0;
    for (loop = 0; loop < qtdeObjetos; loop++)
    {
        if (objetos[loop].peso > capacidade)
            continue;

        valorDoLoop = objetos[loop].valor;
        capacidadeDoLoop = capacidade - objetos[loop].peso;

        valorDoLoop += EncontraValorMaximo(capacidadeDoLoop, qtdeObjetos, objetos);

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

    if ((*qtdeObjetos = LeInteiro(dados)) == ERRO)
        return FALSE;
    
    if ((*objetos = LeObjetos(*qtdeObjetos, dados)) == NULL)
        return FALSE;

    fclose(dados);

    return TRUE;
}

int AbreArquivo(FILE **dados)
{
    FILE *arquivo;
    if ((arquivo = fopen("dados.txt", "r")) == NULL)
    {
        PrintaCabecalhoErro();
        printf("Nao foi possivel abrir o arquivo 'dados.txt'.");
        return FALSE;
    }

    if(feof(arquivo))
    {
        PrintaCabecalhoErro();
        printf("Arquivo 'dados.txt' vazio.");
        return FALSE;
    }

    *dados = arquivo;
    return TRUE;
}

void PrintaInformacoesDoArquivo(int capacidade, int qtdeObjetos)
{
    if (GetIdDaUnidadeDeProcessamento() == 0)
    {
        printf("\n");
        printf("Capacidade da mochila: %d", capacidade);
        printf("\nQuantidade de objetos: %d", qtdeObjetos);
    }
}

void PrintaCabecalhoErro()
{
    printf("\n\n---------- ERRO ----------\n");
}

void PrintaObjetos(int qtdeObjetos, objeto *objetos)
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
        PrintaCabecalhoErro();
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
            PrintaCabecalhoErro();
            printf("Existem mais objetos do que o informado no totalizador.");
            free(objetos);
            return NULL;
        }

        objetos[loop] = TransformarLinhaEmObjeto(linha, loop);

        if (objetos[loop].peso == 0 || objetos[loop].valor == 0)
        {
            PrintaCabecalhoErro();
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

int GetIdDaUnidadeDeProcessamento()
{
    static int id = -1;

    if (id == -1)
        MPI_Comm_rank(MPI_COMM_WORLD, &id);

    return id;
}

int getNumeroDeUnidadesDeProcessamento()
{
    static int size = -1;

    if (size == -1)
        MPI_Comm_size(MPI_COMM_WORLD, &size);

    return size;
}
