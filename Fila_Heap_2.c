#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX 1000

typedef struct {
    int valor;
    int comp_fila;
    int comp_heap;
} Registro;

typedef struct {
    float fila;
    float heap;
} Media;

typedef struct {
    int valor;
    int prioridade;
} Elemento;

typedef struct {
    Elemento itens[MAX];
    int tamanho;
} FilaPrioridadeSimples;

typedef struct {
    Elemento itens[MAX];
    int tamanho;
} FilaPrioridadeComHeap;

void embaralhar(int *vetor, int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = vetor[i];
        vetor[i] = vetor[j];
        vetor[j] = temp;
    }
}

void inserirFilaSimples(FilaPrioridadeSimples *fila, int valor, int prioridade, int *comparacoes) {
    *comparacoes = 0;
    if (fila->tamanho >= MAX) return;
    fila->itens[fila->tamanho++] = (Elemento){valor, prioridade};
}

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

int comparar(const void *a, const void *b) {
    return ((Registro *)a)->valor - ((Registro *)b)->valor;
}

int main() {
    Media medias_insercao = {0, 0}; 
    Media medias_remocao = {0, 0};
    srand(time(NULL));

    int N = rand() % 100 + 400;
    printf("Quantidade de elementos: %d\n", N);

    int valores[MAX];
    for (int i = 0; i < N; i++) valores[i] = i;
     embaralhar(valores, N);

    FilaPrioridadeSimples fila = {.tamanho = 0};
    FilaPrioridadeComHeap heap = {.tamanho = 0};

    Registro insercoes[MAX];
    for (int i = 0; i < N; i++) {
        int valor = valores[i];
        int prioridade = (int)(log2(valor + 2));  // <<< AJUSTE IMPORTANTE

        int compFila = 0, compHeap = 0;

        inserirFilaSimples(&fila, valor, prioridade, &compFila);
        inserirFilaComHeap(&heap, valor, prioridade, &compHeap);

        medias_insercao.fila += compFila;
        medias_insercao.heap += compHeap;

        insercoes[i] = (Registro){valor, compFila, compHeap};
    }

    qsort(insercoes, N, sizeof(Registro), comparar);

    FILE *arqInsercao = fopen("dados_insercao.txt", "w");
    fprintf(arqInsercao, "valor,comparacoes_fila,comparacoes_heap\n");
    for (int i = 0; i < N; i++) {
        fprintf(arqInsercao, "%d,%d,%d\n", insercoes[i].valor, insercoes[i].comp_fila, insercoes[i].comp_heap);
    }
    fclose(arqInsercao);

    printf("Inserção concluída!\nMédia Comparações Fila: %.2f | Heap: %.2f\n",
           medias_insercao.fila / N, medias_insercao.heap / N);

    Registro remocoes[MAX];
    for (int i = 0; i < N; i++) {
        int compFila = 0, compHeap = 0;
        Elemento r1 = removerMaiorPrioridadeSimples(&fila, &compFila);
        Elemento r2 = removerMaiorPrioridadeHeap(&heap, &compHeap);

        medias_remocao.fila += compFila;
        medias_remocao.heap += compHeap;

        remocoes[i] = (Registro){r1.valor, compFila, compHeap};
    }

    qsort(remocoes, N, sizeof(Registro), comparar);

    FILE *arqRemocao = fopen("dados_remocao.txt", "w");
    fprintf(arqRemocao, "valor_removido,comparacoes_fila,comparacoes_heap\n");
    for (int i = 0; i < N; i++) {
        fprintf(arqRemocao, "%d,%d,%d\n", remocoes[i].valor, remocoes[i].comp_fila, remocoes[i].comp_heap);
    }
    fclose(arqRemocao);

    printf("Remoção concluída!\nMédia Comparações Fila: %.2f | Heap: %.2f\n",
           medias_remocao.fila / N, medias_remocao.heap / N);

    return 0;
}
