#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#define BYTE unsigned char

typedef struct No {
    BYTE caractere;
    int frequencia;
    struct No *esquerda, *direita;
} No;

typedef struct {
    BYTE byte;
    int bits;
    BYTE codigo[256];
} Codigo;

typedef struct Heap {
    int tamanho;
    No *dados[256];
} Heap;

typedef struct {
    BYTE byte;
    int frequencia;
} Frequencia;

typedef struct {
    BYTE byte;
    int frequencia;
} Item;

// Funções da árvore de Huffman

No* criar_no(BYTE caractere, int frequencia, No *esquerda, No *direita)
{
    No *novo = (No *) malloc(sizeof(No));
    novo->caractere = caractere;
    novo->frequencia = frequencia;
    novo->esquerda = esquerda;
    novo->direita = direita;
    return novo;
}

int eh_folha(No *no)
{
    return (no->esquerda == NULL && no->direita == NULL);
}

// Funções da heap

Heap* criar_heap()
{
    Heap *heap = (Heap *) malloc(sizeof(Heap));
    heap->tamanho = 0;
    return heap;
}

void trocar(No **a, No **b)
{
    No *temp = *a;
    *a = *b;
    *b = temp;
}

void inserir_heap(Heap *heap, No *no)
{
    int i = heap->tamanho++;
    heap->dados[i] = no;

    while (i && heap->dados[i]->frequencia < heap->dados[(i - 1) / 2]->frequencia)
    {
        trocar(&heap->dados[i], &heap->dados[(i - 1) / 2]);
        i = (i - 1) / 2;
    }
}

No* remover_min(Heap *heap)
{
    No *minimo = heap->dados[0];
    heap->dados[0] = heap->dados[--heap->tamanho];

    int i = 0;
    while (i * 2 + 1 < heap->tamanho)
    {
        int menor = i * 2 + 1;
        if (menor + 1 < heap->tamanho && heap->dados[menor + 1]->frequencia < heap->dados[menor]->frequencia)
            menor++;

        if (heap->dados[i]->frequencia <= heap->dados[menor]->frequencia)
            break;

        trocar(&heap->dados[i], &heap->dados[menor]);
        i = menor;
    }

    return minimo;
}

void contar_frequencias(FILE *arquivo, int *frequencias)
{
    BYTE c;
    while (fread(&c, sizeof(BYTE), 1, arquivo))
    {
        frequencias[c]++;
    }
}

No* construir_arvore(int *frequencias)
{
    Heap *heap = criar_heap();
    for (int i = 0; i < 256; i++)
    {
        if (frequencias[i])
        {
            inserir_heap(heap, criar_no((BYTE)i, frequencias[i], NULL, NULL));
        }
    }

    while (heap->tamanho > 1)
    {
        No *esq = remover_min(heap);
        No *dir = remover_min(heap);
        inserir_heap(heap, criar_no('*', esq->frequencia + dir->frequencia, esq, dir));
    }

    return remover_min(heap);
}

void gerar_codigos(No *raiz, Codigo *tabela, BYTE *codigo, int nivel)
{
    if (raiz == NULL)
        return;

    if (eh_folha(raiz))
    {
        tabela[raiz->caractere].bits = nivel;
        memcpy(tabela[raiz->caractere].codigo, codigo, nivel);
        return;
    }

    codigo[nivel] = 0;
    gerar_codigos(raiz->esquerda, tabela, codigo, nivel + 1);

    codigo[nivel] = 1;
    gerar_codigos(raiz->direita, tabela, codigo, nivel + 1);
}

void escrever_arvore(No *raiz, FILE *out, int *tamanho)
{
    if (raiz == NULL)
        return;

    if (eh_folha(raiz))
    {
        BYTE c = raiz->caractere;
        if (c == '*' || c == '\\')
        {
            BYTE barra = '\\';
            fwrite(&barra, sizeof(BYTE), 1, out);
            (*tamanho)++;
        }
        fwrite(&c, sizeof(BYTE), 1, out);
        (*tamanho)++;
        return;
    }

    BYTE estrela = '*';
    fwrite(&estrela, sizeof(BYTE), 1, out);
    (*tamanho)++;
    escrever_arvore(raiz->esquerda, out, tamanho);
    escrever_arvore(raiz->direita, out, tamanho);
}

void escrever_header(FILE *out, int trash_bits, int tree_size)
{
    unsigned short header = (trash_bits << 13) | tree_size;
    BYTE byte1 = header >> 8;
    BYTE byte2 = header & 0xFF;
    fwrite(&byte1, sizeof(BYTE), 1, out);
    fwrite(&byte2, sizeof(BYTE), 1, out);
}

void ler_header(FILE *in, int *trash_bits, unsigned short *tree_size)
{
    BYTE byte1, byte2;
    fread(&byte1, sizeof(BYTE), 1, in);
    fread(&byte2, sizeof(BYTE), 1, in);
    unsigned short header = (byte1 << 8) | byte2;
    *trash_bits = header >> 13;
    *tree_size = header & 0x1FFF;
}

No* reconstruir_arvore(FILE *in, int *pos)
{
    BYTE c;
    fread(&c, sizeof(BYTE), 1, in);
    (*pos)--;

    if (c == '*')
    {
        No *esq = reconstruir_arvore(in, pos);
        No *dir = reconstruir_arvore(in, pos);
        return criar_no('*', 0, esq, dir);
    }
    else if (c == '\\')
    {
        fread(&c, sizeof(BYTE), 1, in);
        (*pos)--;
    }

    return criar_no(c, 0, NULL, NULL);
}

void compactar_arquivo(const char *entrada, const char *saida)
{
    FILE *in = fopen(entrada, "rb");
    if (!in)
    {
        printf("Erro ao abrir arquivo de entrada\n");
        return;
    }

    int frequencias[256] = {0};
    contar_frequencias(in, frequencias);
    rewind(in);

    No *raiz = construir_arvore(frequencias);
    Codigo tabela[256] = {0};
    BYTE codigo[256];
    gerar_codigos(raiz, tabela, codigo, 0);

    FILE *out = fopen(saida, "wb");
    fseek(out, 2, SEEK_SET);

    int tree_size = 0;
    escrever_arvore(raiz, out, &tree_size);

    BYTE buffer = 0;
    int bits_usados = 0;
    BYTE c;
    while (fread(&c, sizeof(BYTE), 1, in))
    {
        for (int i = 0; i < tabela[c].bits; i++)
        {
            buffer <<= 1;
            if (tabela[c].codigo[i])
                buffer |= 1;
            bits_usados++;

            if (bits_usados == 8)
            {
                fwrite(&buffer, sizeof(BYTE), 1, out);
                bits_usados = 0;
                buffer = 0;
            }
        }
    }

    if (bits_usados > 0) {
        buffer <<= (8 - bits_usados);
        fwrite(&buffer, sizeof(BYTE), 1, out);
    }

    int trash_bits = bits_usados ? 8 - bits_usados : 0;
    rewind(out);
    escrever_header(out, trash_bits, tree_size);

    fclose(in);
    fclose(out);
    printf("Arquivo compactado com sucesso!\n");
}

void descompactar_arquivo(const char *entrada, const char *saida)
{
    FILE *in = fopen(entrada, "rb");
    if (!in)
    {
        printf("Erro ao abrir arquivo de entrada\n");
        return;
    }

    int trash_bits;
    unsigned short tree_size;
    ler_header(in, &trash_bits, &tree_size);

    int pos = tree_size;
    No *raiz = reconstruir_arvore(in, &pos);

    FILE *out = fopen(saida, "wb");
    No *atual = raiz;
    BYTE c;
    long total_bytes = ftell(in);
    fseek(in, 0, SEEK_END);
    long tamanho_total = ftell(in) - total_bytes;
    fseek(in, total_bytes, SEEK_SET);

    for (long i = 0; i < tamanho_total; i++)
    {
        fread(&c, sizeof(BYTE), 1, in);
        for (int j = 7; j >= 0; j--)
        {
            int bit = (c >> j) & 1;
            atual = bit ? atual->direita : atual->esquerda;

            if (eh_folha(atual))
            {
                fwrite(&atual->caractere, sizeof(BYTE), 1, out);
                atual = raiz;
            }

            if (i == tamanho_total - 1 && j == trash_bits)
                break;
        }
    }

    fclose(in);
    fclose(out);
    printf("Arquivo descompactado com sucesso!\n");
}

// Função nova: Verificar header
void verificar_header(const char *arquivo)
{
    FILE *in = fopen(arquivo, "rb");
    if (!in)
    {
        perror("Erro ao abrir arquivo");
        return;
    }

    int trash_bits;
    unsigned short tree_size;
    ler_header(in, &trash_bits, &tree_size);

    printf("Header do arquivo %s:\n", arquivo);
    printf("- Bits de lixo: %d\n", trash_bits);
    printf("- Tamanho da árvore: %hu bytes\n", tree_size);

    fclose(in);
}

// Função nova: Verificar integridade
void verificar_integridade(const char *arquivo_compactado, const char *arquivo_descompactado)
{
    FILE *f1 = fopen(arquivo_compactado, "rb");
    FILE *f2 = fopen(arquivo_descompactado, "rb");

    if (!f1 || !f2)
    {
        printf("Erro ao abrir arquivos para verificação\n");
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return;
    }

    int trash_bits;
    unsigned short tree_size;
    ler_header(f1, &trash_bits, &tree_size);
    fseek(f1, 2 + tree_size, SEEK_SET);

    int erro = 0;
    long offset = 0;
    int c1, c2;

    while ((c1 = fgetc(f1)) != EOF && (c2 = fgetc(f2)) != EOF)
    {
        if (c1 != c2)
        {
            erro = 1;
            break;
        }
        offset++;
    }

    if ((c1 != EOF || c2 != EOF) && !erro)
        erro = 1;

    if (erro)
    {
        printf("Arquivos são diferentes a partir do byte %ld\n", offset);
    }
    else 
    {
        printf("Arquivos são idênticos após a descompactação!\n");
    }

    fclose(f1);
    fclose(f2);
}

// MAIN
int main()
{
    setlocale(LC_ALL, "Portuguese");

    int opcao;
    char nome_arquivo[256] = {0};
    char nome_saida[256] = {0};
    char nome_original[256] = {0};

    printf("Huffman File Compressor\n");
    printf("1. Compactar arquivo\n");
    printf("2. Descompactar arquivo\n");
    printf("3. Verificar header\n");
    printf("Escolha: ");
    if (scanf("%d", &opcao) != 1)
    {
        printf("Entrada inválida\n");
        return 1;
    }
    getchar();

    if (opcao == 1)
    {
        printf("Arquivo a compactar: ");
        fgets(nome_arquivo, sizeof(nome_arquivo), stdin);
        nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';
        strcpy(nome_original, nome_arquivo);
        strcat(nome_arquivo, ".huff");
        compactar_arquivo(nome_original, nome_arquivo);

    }
    else if (opcao == 2)
    {
        printf("Arquivo .huff a descompactar: ");
        fgets(nome_arquivo, sizeof(nome_arquivo), stdin);
        nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';

        if (!strstr(nome_arquivo, ".huff"))
        {
            printf("Deve ser um arquivo .huff\n");
            return 1;
        }

        strncpy(nome_saida, nome_arquivo, strlen(nome_arquivo) - 5);
        nome_saida[strlen(nome_arquivo) - 5] = '\0';
        strcat(nome_saida, ".dehuff");

        descompactar_arquivo(nome_arquivo, nome_saida);

        printf("\nDeseja verificar a integridade? (s/n): ");
        char resposta = getchar();
        if (resposta == 's' || resposta == 'S')
        {
            verificar_integridade(nome_arquivo, nome_saida);
        }

    }
    else if (opcao == 3)
    {
        printf("Arquivo .huff para verificar header: ");
        fgets(nome_arquivo, sizeof(nome_arquivo), stdin);
        nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';
        verificar_header(nome_arquivo);
    }
    else
    {
        printf("Opção inválida!\n");
        return 1;
    }

    return 0;
}
