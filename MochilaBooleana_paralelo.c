/* --------------------------------------------------------------
                PUC Minas - Pocos de Caldas
                     1o semestre 2020
                    Programacao Pararela

            Aluno: Leonardo Oliveira de Moura
            Aluno: Matheus Inhesta

Resolve o problema da mochila booleana de forma iterativa usando forca bruta.
Nao utiliza programacao dinamica.
Utiliza a abordagem de Processor Farm para distribuição das tarefas entre
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

typedef struct
{
    int id;
    int peso;
    int valor;
} objeto;

// veriaveis para versao paralela
int ret, rank, size, tag;

int EncontraValorMaximo(int, int, objeto*);
int LeArquivo(int*, int*, objeto**);
int AbreArquivo(FILE **);
void printaCabecalhoErro();
void printaObjetos(int, objeto*);
int LeInteiro(FILE*);
objeto* LeObjetos(int, FILE*);
objeto TransformarLinhaEmObjeto(char*, int);
char **SplitString(char*);


int main(int argc, char *argv[])
{
    int loop, valorMaximo;
    int capacidade, qtdeObjetos;
    clock_t inicio, fim;
    objeto *objetos;


    ret = MPI_Init(&argc, &argv);
    ret = MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    ret = MPI_Comm_size(MPI_COMM_WORLD, &size);
    tag = 2587;


    if (!LeArquivo(&capacidade, &qtdeObjetos, &objetos))
        return 1;

    printf("\n\nRealizando a busca de combinacoes.");
    inicio = clock();
    valorMaximo = EncontraValorMaximo(capacidade, qtdeObjetos, objetos);
    fim = clock();
	if(rank == 0){
	    printf("\nValor maximo suportado: %d", valorMaximo);
	    printf("\nTempo gasto em milissegundos: %g", ((double)(fim - inicio)) * 1000.0 / CLOCKS_PER_SEC);
	}

    getchar();
}

int EncontraValorMaximo(int capacidade, int qtdeObjetos, objeto *objetos)
{
    int loop, valorMaximo, valorDoLoop, capacidadeDoLoop, valorParalelo, valorMaximoParalelo;
    
    MPI_Status status;

    valorMaximo = 0;
    valorDoLoop = 0;
    capacidadeDoLoop = 0;
    loop = 0;

    while(loop < qtdeObjetos) {

        if (objetos[loop+rank].peso > capacidade)
            continue;

        valorDoLoop = objetos[loop+rank].valor;
        capacidadeDoLoop = capacidade - objetos[loop+rank].peso;

        valorDoLoop += EncontraValorMaximo(capacidadeDoLoop, qtdeObjetos, objetos);

        if (valorDoLoop > valorMaximo)
            valorMaximo = valorDoLoop;
	  
	// size é o numero de nós
        loop += size;
    }

    // Envia pro nó zero o valorMaximo
    if(rank != 0){
	printf("Envio pro rank 0 para %d", rank);
	ret = MPI_Send(&valorMaximo, 1, MPI_INT, 0, tag, MPI_COMM_WORLD);
    } else {
	printf("entrou rank 0");
	loop = 1; // pula o nó 0
	valorDoLoop = 0;
	valorParalelo = 0;
	valorMaximoParalelo = 0;
	
	while(loop < size){
		ret = MPI_Recv(&valorParalelo, 1, MPI_INT, loop, tag, MPI_COMM_WORLD, &status);

		if(valorParalelo > valorMaximoParalelo)
			valorMaximoParalelo = valorParalelo;

		loop++;
	}

	if(valorMaximoParalelo > valorMaximo)
		valorMaximo = valorMaximoParalelo;
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
