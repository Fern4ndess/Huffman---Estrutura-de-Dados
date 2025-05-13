#include <stdio.h>      // Biblioteca padrão para entrada e saída (printf, fopen, etc)
#include <stdlib.h>     // Biblioteca padrão para funções como rand, malloc, free
#include <time.h>       // Biblioteca usada para manipular tempo (seed do rand)
#include <math.h>       // Biblioteca matemática (utilizada para log2)

#define MAX 1000        // Define o tamanho máximo das filas de prioridade

/**
 * @struct Registro
 * @brief Estrutura que armazena o resultado de uma operação de remoção.
 *
 * Contém o valor do elemento removido e o número de comparações realizadas
 * em duas diferentes implementações de filas de prioridade:
 * - uma fila de prioridade simples 
 * - um heap binário (máx-heap)
 */
typedef struct {
    int valor;          ///< Valor do elemento removido
    int comp_fila;      ///< Quantidade de comparações feitas na fila simples
    int comp_heap;      ///< Quantidade de comparações feitas na fila com heap
} Registro;


// Estrutura representando um elemento com valor e prioridade
typedef struct {
    int valor;          // Valor do elemento
    int prioridade;     // Prioridade associada ao elemento
} Elemento;

// Estrutura da fila de prioridade simples (implementada como vetor comum)
typedef struct {
    Elemento itens[MAX]; // Vetor que armazena os elementos
    int tamanho;         // Quantidade atual de elementos na fila
} FilaPrioridadeSimples;

// Estrutura da fila de prioridade com heap (máx-heap)
typedef struct {
    Elemento itens[MAX]; // Vetor que armazena os elementos em forma de heap
    int tamanho;         // Quantidade atual de elementos no heap
} FilaPrioridadeComHeap;

/**
 * @brief Embaralha os elementos de um vetor de inteiros.
 * 
 * Implementa o algoritmo Fisher-Yates Shuffle (também conhecido como 
 * Knuth Shuffle), que é um algoritmo clássico para embaralhar vetores 
 * de forma justa e uniforme.
 * 
 * O algoritmo percorre o vetor de trás para frente, e para cada posição i, 
 * escolhe aleatoriamente um índice j entre 0 e i, trocando os elementos das 
 * posições i e j. Isso garante que todas as possíveis permutações tenham 
 * a mesma probabilidade de ocorrer — ou seja, é um embaralhamento "justo".
 *
 * Essa aleatoriedade é **essencial para testes comparativos de desempenho** 
 * entre estruturas de dados, como filas de prioridade simples e heaps. 
 * Sem ela, os dados estariam sempre em ordem crescente, o que pode gerar 
 * **comportamento artificialmente otimizado ou pessimizado**, influenciando 
 * as comparações de forma irreal.
 *
 * Embora o vetor seja reordenado depois da remoção (com `qsort`), 
 * o embaralhamento anterior garante que os elementos foram inseridos 
 * de forma **não enviesada**, simulando um cenário realista onde os dados 
 * chegam em ordem aleatória. Isso torna o teste **justo e estatisticamente confiável**.
 * 
 * @param vetor Vetor de inteiros a ser embaralhado
 * @param n Tamanho do vetor
 */
void embaralhar(int *vetor, int n) {
    for (int i = n - 1; i > 0; i--) {               // Percorre de trás para frente
        int j = rand() % (i + 1);                   // Escolhe um índice aleatório entre 0 e i
        int temp = vetor[i];                        // Troca o elemento atual com o aleatório
        vetor[i] = vetor[j];
        vetor[j] = temp;
    }
}


// Função que remove o elemento de maior prioridade da fila simples
Elemento removerMaiorPrioridadeSimples(FilaPrioridadeSimples *fila, int *comparacoes) {
    *comparacoes = 0;                             // Zera o contador de comparações
    int idx = 0;                                  // Inicializa o índice do maior elemento

    // Busca o índice do maior elemento baseado na prioridade
    for (int i = 1; i < fila->tamanho; i++) {
        (*comparacoes)++;                         // Conta a comparação
        if (fila->itens[i].prioridade > fila->itens[idx].prioridade) {
            idx = i;                              // Atualiza o índice do maior
        }
    }

    Elemento removido = fila->itens[idx];         // Armazena o elemento a ser removido

    // Desloca os elementos após idx uma posição para a esquerda
    for (int i = idx; i < fila->tamanho - 1; i++) {
        fila->itens[i] = fila->itens[i + 1];
    }

    fila->tamanho--;                              // Decrementa o tamanho da fila
    return removido;                              // Retorna o elemento removido
}

// Função que remove o elemento de maior prioridade do heap (máx-heap)
Elemento removerMaiorPrioridadeHeap(FilaPrioridadeComHeap *heap, int *comparacoes) {
    *comparacoes = 0;                              // Zera o contador de comparações

    if (heap->tamanho == 0)                        // Verifica se o heap está vazio
        return (Elemento){-1, -1};                 // Retorna valor inválido

    Elemento removido = heap->itens[0];            // Raiz é o maior elemento
    heap->itens[0] = heap->itens[--heap->tamanho]; // Substitui a raiz pelo último elemento

    int i = 0;                                     // Começa na raiz
    while (1) {
        int esq = 2 * i + 1;                       // Índice do filho esquerdo
        int dir = 2 * i + 2;                       // Índice do filho direito
        int maior = i;                             // Assume inicialmente que o maior é o atual

        // Verifica se o filho esquerdo tem maior prioridade
        if (esq < heap->tamanho) {
            (*comparacoes)++;
            if (heap->itens[esq].prioridade > heap->itens[maior].prioridade)
                maior = esq;
        }

        // Verifica se o filho direito tem maior prioridade
        if (dir < heap->tamanho) {
            (*comparacoes)++;
            if (heap->itens[dir].prioridade > heap->itens[maior].prioridade)
                maior = dir;
        }

        if (maior == i) break;                     // Se já estiver na posição correta, para

        // Troca o atual com o maior filho
        Elemento tmp = heap->itens[i];
        heap->itens[i] = heap->itens[maior];
        heap->itens[maior] = tmp;

        i = maior;                                 // Continua a verificação no novo índice
    }

    return removido;                               // Retorna o elemento removido
}

// Função de comparação usada pelo qsort para ordenar registros por valor
int comparar(const void *a, const void *b) {
    return ((Registro *)a)->valor - ((Registro *)b)->valor;
}

// Função principal
int main() {
    srand(time(NULL));                             // Inicializa a semente aleatória para o rand()

    int N = rand() % 500 + 500;                    // Gera um número aleatório entre 500 e 999
    printf("Quantidade de elementos: %d\n", N);    // Imprime a quantidade de elementos

    int valores[MAX];
    for (int i = 0; i < N; i++) valores[i] = i;    // Preenche o vetor com valores de 0 a N-1
    embaralhar(valores, N);                       // Embaralha os valores para simular dados aleatórios

    FilaPrioridadeSimples fila = {.tamanho = 0};   // Inicializa a fila simples
    FilaPrioridadeComHeap heap = {.tamanho = 0};   // Inicializa o heap

    // Preenche ambas as estruturas com os mesmos dados (valor e prioridade)
    for (int i = 0; i < N; i++) {
        /**
         * Geração da prioridade com base no valor do elemento, usando logaritmo base 2.
         *
         * A função log2() retorna o logaritmo de base 2 do argumento. Aqui usamos:
         *   log2(valor + 2)
         *
         * - A adição de 2 serve para evitar valores inválidos, como log2(0), que é indefinido,
         *   e log2(1), que resultaria em 0 (sem distinção de prioridade).
         * - O uso do logaritmo cria uma distribuição *não linear* de prioridades:
         *   os valores menores têm saltos maiores entre si, enquanto os maiores se aproximam.
         *   Ex: log2(2) = 1, log2(4) = 2, log2(8) = 3, ..., log2(1024) = 10.
         *
         * Isso simula cenários realistas onde múltiplos elementos podem acabar com a mesma
         * prioridade (após truncar com cast para `int`), forçando as estruturas a lidar
         * com empates — o que é útil para testar a estabilidade e eficiência do algoritmo.
         */
        int prioridade = (int)(log2(valores[i] + 2));  // Exemplo: log2(0 + 2) = 1

        fila.itens[fila.tamanho++] = (Elemento){valores[i], prioridade}; // Insere na fila
        heap.itens[heap.tamanho++] = (Elemento){valores[i], prioridade}; // Insere no heap
    }

    Registro remocoes[MAX];         // Vetor para armazenar os dados das remoções

    // Loop de remoção
    for (int i = 0; i < N; i++) {
        int compFila = 0, compHeap = 0;            // Contadores de comparações

        Elemento r1 = removerMaiorPrioridadeSimples(&fila, &compFila); // Remove da fila simples
        Elemento r2 = removerMaiorPrioridadeHeap(&heap, &compHeap);    // Remove do heap

        remocoes[i] = (Registro){r1.valor, compFila, compHeap};       // Armazena as comparações
    }

    qsort(remocoes, N, sizeof(Registro), comparar); // Ordena os registros por valor para facilitar visualização

    // Escreve os dados em um arquivo CSV para visualização posterior (ex: em MATLAB)
    FILE *arqRemocao = fopen("dados_remocao.txt", "w");
    fprintf(arqRemocao, "valor_removido,comparacoes_fila,comparacoes_heap\n");
    for (int i = 0; i < N; i++) {
        fprintf(arqRemocao, "%d,%d,%d\n", remocoes[i].valor, remocoes[i].comp_fila, remocoes[i].comp_heap);
    }
    fclose(arqRemocao);                            // Fecha o arquivo

    return 0;                                       // Fim do programa
}
