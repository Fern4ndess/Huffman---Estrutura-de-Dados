#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX 1000

typedef struct {
    float fila;
    float heap;
} Media;

typedef struct {
    int valor;
    int prioridade;
} Elemento;

// Fila de Prioridade sem Heap (implementação simples)
typedef struct {
    Elemento itens[MAX];
    int tamanho;
} FilaPrioridadeSimples;

// Fila de Prioridade com Heap (implementação com heap binário)
typedef struct {
    Elemento itens[MAX];
    int tamanho;
} FilaPrioridadeComHeap;

// Embaralha os valores de um vetor (Fisher-Yates)
void embaralhar(int *vetor, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = vetor[i];
        vetor[i] = vetor[j];
        vetor[j] = temp;
    }
}

// Inserção sem comparações (fila simples)
void inserirFilaSimples(FilaPrioridadeSimples *fila, int valor, int prioridade, int *comparacoes) {
    *comparacoes = 0; // inserção direta
    if (fila->tamanho >= MAX) return;
    fila->itens[fila->tamanho++] = (Elemento){valor, prioridade};
}

// Inserção com heapify-up (fila com heap)
void inserirFilaComHeap(FilaPrioridadeComHeap *heap, int valor, int prioridade, int *comparacoes) {
    if (heap->tamanho >= MAX) return;
    int i = heap->tamanho++;
    heap->itens[i] = (Elemento){valor, prioridade};
    *comparacoes = 0;

    while (i > 0) {
        int pai = (i - 1) / 2;
        (*comparacoes)++;
        if (heap->itens[i].prioridade <= heap->itens[pai].prioridade) break;

        Elemento tmp = heap->itens[i];
        heap->itens[i] = heap->itens[pai];
        heap->itens[pai] = tmp;
        i = pai;
    }
}

// Remoção da maior prioridade na fila simples
Elemento removerMaiorPrioridadeSimples(FilaPrioridadeSimples *fila, int *comparacoes) {
    *comparacoes = 0;
    int idx = 0;
    for (int i = 1; i < fila->tamanho; i++) {
        (*comparacoes)++;
        if (fila->itens[i].prioridade > fila->itens[idx].prioridade) {
            idx = i;
        }
    }
    Elemento removido = fila->itens[idx];
    for (int i = idx; i < fila->tamanho - 1; i++) {
        fila->itens[i] = fila->itens[i + 1];
    }
    fila->tamanho--;
    return removido;
}

// Remoção da raiz na heap com heapify-down
Elemento removerMaiorPrioridadeHeap(FilaPrioridadeComHeap *heap, int *comparacoes) {
    *comparacoes = 0;
    if (heap->tamanho == 0) return (Elemento){-1, -1};

    Elemento removido = heap->itens[0];
    heap->itens[0] = heap->itens[--heap->tamanho];

    int i = 0;
    while (1) {
        int esq = 2 * i + 1;
        int dir = 2 * i + 2;
        int maior = i;

        if (esq < heap->tamanho) {
            (*comparacoes)++;
            if (heap->itens[esq].prioridade > heap->itens[maior].prioridade)
                maior = esq;
        }

        if (dir < heap->tamanho) {
            (*comparacoes)++;
            if (heap->itens[dir].prioridade > heap->itens[maior].prioridade)
                maior = dir;
        }

        if (maior == i) break;

        Elemento tmp = heap->itens[i];
        heap->itens[i] = heap->itens[maior];
        heap->itens[maior] = tmp;
        i = maior;
    }

    return removido;
}

int main() {
    Media medias_insercao = {0, 0}; 
    Media medias_remocao = {0, 0};
    
    srand(time(NULL));

    int N = rand() % 1000 + 25; // Entre 25 e 124
    printf("Quantidade de elementos: %d\n", N);

    // Vetor de valores únicos embaralhados
    int valores[MAX];
    for (int i = 0; i < N; i++) valores[i] = i;
    embaralhar(valores, N);

    // Estruturas
    FilaPrioridadeSimples fila = {.tamanho = 0};
    FilaPrioridadeComHeap heap = {.tamanho = 0};

    // Arquivo de inserção
    FILE *arqInsercao = fopen("dados_insercao.txt", "w");
    fprintf(arqInsercao, "valor,comparacoes_fila,comparacoes_heap\n");

    for (int i = 0; i < N; i++) {
        int valor = valores[i];
        int prioridade = rand() % 10 + 1;

        int compFila = 0, compHeap = 0;

        inserirFilaSimples(&fila, valor, prioridade, &compFila);
        inserirFilaComHeap(&heap, valor, prioridade, &compHeap);
        
        medias_insercao.fila += compFila;
        medias_insercao.heap += compHeap;

        fprintf(arqInsercao, "%d,%d,%d\n", valor, compFila, compHeap);
    }

    fclose(arqInsercao);
    printf("Inserção concluída! Dados salvos em 'dados_insercao.txt'\n");
    printf("Média de Comparação da Fila: %.2f\n", medias_insercao.fila / N);
    printf("Média de Comparação do Heap: %.2f\n", medias_insercao.heap / N);

    // Arquivo de remoção
    FILE *arqRemocao = fopen("dados_remocao.txt", "w");
    fprintf(arqRemocao, "valor_removido,comparacoes_fila,comparacoes_heap\n");

    for (int i = 0; i < N; i++) {
        int compFila = 0, compHeap = 0;
        
        Elemento r1 = removerMaiorPrioridadeSimples(&fila, &compFila);
        Elemento r2 = removerMaiorPrioridadeHeap(&heap, &compHeap);
        
        medias_insercao.fila += compFila;
        medias_insercao.heap += compHeap;
        
        fprintf(arqRemocao, "%d,%d,%d\n", r1.valor, compFila, compHeap);
    }

    fclose(arqRemocao);
    printf("Remoção concluída! Dados salvos em 'dados_remocao.txt'\n");
    printf("Média de Comparação da Fila: %.2f\n", medias_insercao.fila / N);
    printf("Média de Comparação do Heap: %.2f\n", medias_insercao.heap / N);

    return 0;
}
