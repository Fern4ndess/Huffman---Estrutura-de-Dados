#include <stdio.h>      // Biblioteca padrão de entrada e saída
#include <stdlib.h>     // Biblioteca para funções utilitárias (rand, malloc, etc)
#include <time.h>       // Biblioteca para manipular tempo (utilizada para gerar seed aleatória)
#include <math.h>       // Biblioteca matemática (utilizada para log2)

#define MAX 1000        // Define o número máximo de elementos permitidos nas filas

// Estrutura para registrar dados de inserção e remoção
typedef struct {
    int valor;          // Valor do elemento
    int comp_fila;      // Comparações feitas na fila simples
    int comp_heap;      // Comparações feitas no heap
} Registro;

// Estrutura de um elemento com valor e prioridade
typedef struct {
    int valor;          // Valor do elemento
    int prioridade;     // Prioridade associada
} Elemento;

// Estrutura da Fila de Prioridade Simples (vetor sem heap)
typedef struct {
    Elemento itens[MAX]; // Vetor de elementos
    int tamanho;         // Número de elementos atuais na fila
} FilaPrioridadeSimples;

// Estrutura da Fila de Prioridade com Heap (máx-heap)
typedef struct {
    Elemento itens[MAX]; // Vetor de elementos
    int tamanho;         // Número de elementos no heap
} FilaPrioridadeComHeap;

// Função para embaralhar um vetor de inteiros (algoritmo de Fisher-Yates)
void embaralhar(int *vetor, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);      // Gera índice aleatório
        int temp = vetor[i];           // Troca os elementos
        vetor[i] = vetor[j];
        vetor[j] = temp;
    }
}

// Remove o elemento de maior prioridade da fila simples (varre todos os elementos)
Elemento removerMaiorPrioridadeSimples(FilaPrioridadeSimples *fila, int *comparacoes) {
    *comparacoes = 0;  // Zera comparações
    int idx = 0;       // Índice do maior até agora
    for (int i = 1; i < fila->tamanho; i++) {
        (*comparacoes)++;  // Compara prioridade
        if (fila->itens[i].prioridade > fila->itens[idx].prioridade) {
            idx = i;  // Atualiza índice do maior
        }
    }
    Elemento removido = fila->itens[idx];  // Elemento a ser removido

    // Move os elementos seguintes uma posição à esquerda
    for (int i = idx; i < fila->tamanho - 1; i++) {
        fila->itens[i] = fila->itens[i + 1];
    }
    fila->tamanho--;  // Reduz o tamanho da fila
    return removido;
}

// Remove o elemento de maior prioridade do heap (raiz)
Elemento removerMaiorPrioridadeHeap(FilaPrioridadeComHeap *heap, int *comparacoes) {
    *comparacoes = 0;
    if (heap->tamanho == 0) return (Elemento){-1, -1};  // Heap vazio

    Elemento removido = heap->itens[0];  // Raiz é o maior elemento
    heap->itens[0] = heap->itens[--heap->tamanho];  // Substitui a raiz pelo último

    int i = 0;
    while (1) {
        int esq = 2 * i + 1;  // Índice do filho esquerdo
        int dir = 2 * i + 2;  // Índice do filho direito
        int maior = i;

        // Verifica se filho esquerdo é maior que o atual
        if (esq < heap->tamanho) {
            (*comparacoes)++;
            if (heap->itens[esq].prioridade > heap->itens[maior].prioridade)
                maior = esq;
        }

        // Verifica se filho direito é maior que o maior até agora
        if (dir < heap->tamanho) {
            (*comparacoes)++;
            if (heap->itens[dir].prioridade > heap->itens[maior].prioridade)
                maior = dir;
        }

        if (maior == i) break;  // Se já estiver em posição, para

        // Troca com o maior filho
        Elemento tmp = heap->itens[i];
        heap->itens[i] = heap->itens[maior];
        heap->itens[maior] = tmp;
        i = maior;  // Continua descendo
    }

    return removido;
}

// Função de comparação para qsort (ordena por valor)
int comparar(const void *a, const void *b) {
    return ((Registro *)a)->valor - ((Registro *)b)->valor;
}

// Função principal do programa
int main() {
    srand(time(NULL)); // Seed aleatória

    int N = rand() % 500 + 500;
    printf("Quantidade de elementos: %d\n", N);

    int valores[MAX];
    for (int i = 0; i < N; i++) valores[i] = i;
    embaralhar(valores, N);

    FilaPrioridadeSimples fila = {.tamanho = 0};
    FilaPrioridadeComHeap heap = {.tamanho = 0};

    // Inserção dos elementos nas estruturas
    for (int i = 0; i < N; i++) {
        int prioridade = (int)(log2(valores[i] + 2));
        fila.itens[fila.tamanho++] = (Elemento){valores[i], prioridade};
        heap.itens[heap.tamanho++] = (Elemento){valores[i], prioridade};
    }

    Registro remocoes[MAX];

    // Remoção
    for (int i = 0; i < N; i++) {
        int compFila = 0, compHeap = 0;
        Elemento r1 = removerMaiorPrioridadeSimples(&fila, &compFila);
        Elemento r2 = removerMaiorPrioridadeHeap(&heap, &compHeap);
        remocoes[i] = (Registro){r1.valor, compFila, compHeap};
    }

    qsort(remocoes, N, sizeof(Registro), comparar);

    FILE *arqRemocao = fopen("dados_remocao.txt", "w");
    fprintf(arqRemocao, "valor_removido,comparacoes_fila,comparacoes_heap\n");
    for (int i = 0; i < N; i++) {
        fprintf(arqRemocao, "%d,%d,%d\n", remocoes[i].valor, remocoes[i].comp_fila, remocoes[i].comp_heap);
    }
    fclose(arqRemocao);

    return 0;
}