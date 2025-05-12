#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

/**
 * Define um apelido para o tipo de dado unsigned char.
 *
 * Representa um byte (valores de 0 a 255).
 * Usado para melhorar a legibilidade do código quando se deseja 
 * trabalhar diretamente com dados binários ou caracteres como bytes puros.
 *
 * @define BYTE Tipo que representa um único byte (unsigned char)
 */
#define BYTE unsigned char

/**
 * Estrutura que representa um nó da árvore de Huffman.
 * 
 * Cada nó pode representar:
 * 
 * - Uma folha contendo um caractere e sua frequência
 * - Um nó interno com ponteiros para os filhos esquerdo e direito
 * 
 * @field caractere   Caractere representado (válido apenas em folhas)
 * @field frequencia  Frequência de ocorrência do caractere
 * @field esquerda    Ponteiro para o filho à esquerda na árvore
 * @field direita     Ponteiro para o filho à direita na árvore
 */
typedef struct No
{
    BYTE caractere;
    int frequencia;
    struct No *esquerda, *direita;
} No;

/**
 * Estrutura que representa o código de Huffman gerado para um caractere.
 *
 * Cada caractere é associado a um caminho na árvore de Huffman, codificado
 * como uma sequência de bits ('0' e '1'), que são armazenados como bytes.
 *
 * @field byte     Caractere que será codificado (por exemplo: 'A', 'B', etc.)
 * @field bits     Número de bits usados no código de Huffman para o caractere
 *                 Exemplo: se o código for "101", então bits = 3
 * @field codigo   Vetor de bytes que representa o código de Huffman para o caractere.
 *                 Por simplicidade, pode armazenar os bits como caracteres '0' e '1'.
 *                 Tamanho máximo de 256 posições.
 */
typedef struct
{
    BYTE byte;
    int bits;
    BYTE codigo[256];
} Codigo;

/**
 * Estrutura que define uma min-heap (fila de prioridade) utilizada
 * na construção da árvore de Huffman.
 *
 * A heap armazena nós da árvore e os mantém ordenados pela frequência:
 * o nó com menor frequência tem maior prioridade.
 *
 * @field tamanho  Número atual de elementos na heap.
 * @field dados    Vetor de ponteiros para nós (struct No). Cada posição contém
 *                 um nó da árvore de Huffman. Capacidade máxima de 256 elementos.
 */
typedef struct Heap
{
    int tamanho;
    No *dados[256];
} Heap;

/**
 * Estrutura que armazena a frequência de ocorrência de um caractere em um arquivo.
 *
 * Utilizada para registrar quantas vezes cada caractere (0 a 255) aparece, 
 * servindo como base para a construção da árvore de Huffman.
 *
 * @field byte        Caractere (de 0 a 255) cuja frequência está sendo contada.
 * @field frequencia  Quantidade de vezes que o caractere apareceu no arquivo.
 */
typedef struct
{
    BYTE byte;
    int frequencia;
} Frequencia;

/**
 * Estrutura que representa um caractere e sua frequência de ocorrência,
 * usada em contextos diferentes da struct Frequencia, como serialização
 * da árvore de Huffman ou ordenações específicas durante a compressão.
 *
 * Apesar de ser idêntica em estrutura à Frequencia, foi separada para
 * fins de organização semântica no código.
 *
 * @field byte        Caractere (valor de 0 a 255).
 * @field frequencia  Número de ocorrências do caractere.
 */
typedef struct
{
    BYTE byte;
    int frequencia;
} Item;

/**
 * Cria um novo nó para a árvore de Huffman.
 *
 * Inicializa um nó com o caractere, sua frequência e os ponteiros
 * para os filhos esquerdo e direito.
 *
 * @param caractere  Caractere a ser armazenado no nó.
 * @param frequencia Frequência de ocorrência do caractere.
 * @param esquerda   Ponteiro para o filho à esquerda (pode ser NULL).
 * @param direita    Ponteiro para o filho à direita (pode ser NULL).
 * @return Ponteiro para o novo nó alocado dinamicamente.
 */
No* criar_no(BYTE caractere, int frequencia, No *esquerda, No *direita)
{
    No *novo = (No *) malloc(sizeof(No));
    novo->caractere = caractere;
    novo->frequencia = frequencia;
    novo->esquerda = esquerda;
    novo->direita = direita;
    return novo;
}

/**
 * Verifica se um nó da árvore de Huffman é uma folha.
 *
 * Um nó é considerado folha se não possui filhos esquerdo nem direito.
 *
 * @param no Ponteiro para o nó a ser verificado.
 * @return 1 se o nó for uma folha, 0 caso contrário.
 */
int eh_folha(No *no)
{
    return (no->esquerda == NULL && no->direita == NULL);
}

/**
 * Cria e inicializa uma nova heap vazia para uso na construção da árvore de Huffman.
 *
 * A heap é alocada dinamicamente e seu tamanho inicial é zero.
 *
 * @return Ponteiro para a heap recém-criada.
 */
Heap* criar_heap()
{
    Heap *heap = (Heap *) malloc(sizeof(Heap));
    heap->tamanho = 0;
    return heap;
}

/**
 * Troca os ponteiros de dois nós da árvore de Huffman.
 *
 * Útil para operações de ordenação ou reorganização na heap.
 *
 * @param a Ponteiro para o primeiro ponteiro de nó.
 * @param b Ponteiro para o segundo ponteiro de nó.
 */
void trocar(No **a, No **b)
{
    No *temp = *a;
    *a = *b;
    *b = temp;
}

/**
 * Insere um nó na heap (fila de prioridade mínima) mantendo a propriedade da min-heap.
 *
 * O nó é adicionado no final da heap e "subido" até a posição correta com base na frequência,
 * garantindo que o menor elemento (menor frequência) fique na raiz.
 *
 * @param heap Ponteiro para a heap onde o nó será inserido.
 * @param no   Ponteiro para o nó a ser inserido.
 */
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

/**
 * Remove e retorna o nó com menor frequência da heap (raiz da min-heap).
 *
 * Após remover o menor elemento (raiz), a heap é reorganizada para manter
 * a propriedade da min-heap, realizando o rebaixamento (heapify down).
 *
 * @param heap Ponteiro para a heap de onde o menor nó será removido.
 * @return Ponteiro para o nó com menor frequência.
 */
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

/**
 * Conta a frequência de ocorrência de cada caractere em um arquivo.
 *
 * Lê o arquivo byte por byte e incrementa a contagem no vetor de frequências
 * correspondente ao valor do caractere (0 a 255).
 *
 * @param arquivo     Ponteiro para o arquivo a ser lido.
 * @param frequencias Vetor de 256 posições onde cada índice representa
 *                    um caractere e seu valor representa a quantidade de vezes
 *                    que ele apareceu.
 */
void contar_frequencias(FILE *arquivo, int *frequencias)
{
    BYTE c;
    while (fread(&c, sizeof(BYTE), 1, arquivo))
    {
        frequencias[c]++;
    }
}

/**
 * Constrói a árvore de Huffman com base nas frequências dos caracteres.
 *
 * Cria uma heap mínima e insere um nó para cada caractere com frequência não nula.
 * Em seguida, combina os dois nós de menor frequência repetidamente, formando
 * a árvore de Huffman. Cada novo nó criado representa a união de dois nós filhos.
 *
 * @param frequencias Vetor de 256 posições contendo a frequência de cada caractere.
 * @return Ponteiro para a raiz da árvore de Huffman construída.
 */
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

/**
 * Gera os códigos de Huffman para cada caractere, percorrendo a árvore recursivamente.
 *
 * Para cada folha da árvore (caractere), armazena a sequência de bits que representa
 * o caminho da raiz até aquela folha, usando 0 para esquerda e 1 para direita.
 *
 * @param raiz    Ponteiro para o nó atual da árvore de Huffman.
 * @param tabela  Vetor de 256 posições onde será armazenado o código de cada caractere.
 * @param codigo  Vetor temporário usado para construir o caminho atual (sequência de bits).
 * @param nivel   Profundidade atual na árvore (quantidade de bits do código em construção).
 */
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

/**
 * Serializa a árvore de Huffman e grava no arquivo de saída.
 *
 * A função percorre a árvore de Huffman e grava cada caractere e cada nó interno
 * (representado por '*'). Se o caractere for especial ('*' ou '\\'), é necessário
 * escapar o caractere com uma barra invertida antes de gravá-lo.
 *
 * @param raiz    Ponteiro para a raiz da árvore de Huffman a ser serializada.
 * @param out     Ponteiro para o arquivo de saída onde a árvore será escrita.
 * @param tamanho Ponteiro para a variável que mantém o tamanho atual da árvore escrita no arquivo.
 */
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

/**
 * Escreve o cabeçalho do arquivo comprimido, contendo informações sobre o número de bits de lixo
 * e o tamanho da árvore de Huffman.
 *
 * O cabeçalho é composto por dois bytes: os 13 bits mais significativos armazenam o número de bits
 * de lixo (bits não usados no último byte) e os 3 bits menos significativos armazenam o tamanho da
 * árvore de Huffman.
 *
 * @param out        Ponteiro para o arquivo de saída onde o cabeçalho será escrito.
 * @param trash_bits Número de bits de lixo usados no último byte do arquivo comprimido.
 * @param tree_size  Tamanho da árvore de Huffman, em bytes.
 */
void escrever_header(FILE *out, int trash_bits, int tree_size)
{
    unsigned short header = (trash_bits << 13) | tree_size;
    BYTE byte1 = header >> 8;
    BYTE byte2 = header & 0xFF;
    fwrite(&byte1, sizeof(BYTE), 1, out);
    fwrite(&byte2, sizeof(BYTE), 1, out);
}

/**
 * Lê o cabeçalho do arquivo comprimido, extraindo o número de bits de lixo e o tamanho da árvore de Huffman.
 *
 * O cabeçalho é composto por dois bytes, onde os 13 bits mais significativos armazenam o número de bits
 * de lixo e os 3 bits menos significativos armazenam o tamanho da árvore de Huffman.
 *
 * @param in          Ponteiro para o arquivo de entrada do qual o cabeçalho será lido.
 * @param trash_bits  Ponteiro para variável onde será armazenado o número de bits de lixo.
 * @param tree_size   Ponteiro para variável onde será armazenado o tamanho da árvore de Huffman, em bytes.
 */
void ler_header(FILE *in, int *trash_bits, unsigned short *tree_size)
{
    BYTE byte1, byte2;
    fread(&byte1, sizeof(BYTE), 1, in);
    fread(&byte2, sizeof(BYTE), 1, in);
    unsigned short header = (byte1 << 8) | byte2;
    *trash_bits = header >> 13;
    *tree_size = header & 0x1FFF;
}

/**
 * Reconstrói a árvore de Huffman a partir de um arquivo de entrada.
 *
 * A função lê recursivamente o arquivo, reconstruindo a árvore de Huffman a partir dos dados serializados.
 * Para cada nó interno (representado por '*'), a função chama-se recursivamente para reconstruir os
 * subárvores esquerda e direita. Para os nós folha, o caractere é diretamente lido do arquivo.
 *
 * @param in     Ponteiro para o arquivo de entrada de onde a árvore será reconstruída.
 * @param pos    Ponteiro para a posição atual no arquivo (utilizado para controle de leitura).
 * @return       Ponteiro para a raiz da árvore de Huffman reconstruída.
 */
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

/**
 * Compacta um arquivo usando o algoritmo de Huffman e grava o resultado em outro arquivo.
 *
 * A função realiza a compactação de um arquivo de entrada gerando uma árvore de Huffman
 * para os caracteres do arquivo, cria os códigos binários correspondentes para cada caractere,
 * e escreve os dados comprimidos no arquivo de saída. O cabeçalho do arquivo comprimido
 * contém informações sobre o número de bits de lixo e o tamanho da árvore de Huffman.
 *
 * @param entrada Caminho para o arquivo de entrada a ser compactado.
 * @param saida   Caminho para o arquivo de saída onde os dados comprimidos serão gravados.
 */
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

/**
 * Descompacta um arquivo comprimido usando o algoritmo de Huffman e grava o conteúdo no arquivo de saída.
 *
 * A função realiza a descompactação de um arquivo comprimido, reconstruindo a árvore de Huffman
 * a partir do cabeçalho e dos dados serializados. A partir dessa árvore, ela percorre os bits comprimidos
 * para restaurar os caracteres originais e gravá-los no arquivo de saída.
 * O cabeçalho do arquivo comprimido contém informações sobre o número de bits de lixo e o tamanho da árvore.
 *
 * @param entrada Caminho para o arquivo comprimido a ser descompactado.
 * @param saida   Caminho para o arquivo de saída onde os dados descompactados serão gravados.
 */
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

/**
 * Verifica e exibe o cabeçalho de um arquivo comprimido.
 *
 * A função abre o arquivo de entrada, lê e exibe o cabeçalho, que contém
 * informações sobre o número de bits de lixo e o tamanho da árvore de Huffman
 * usada para a compressão. Essas informações ajudam a entender o formato
 * do arquivo comprimido, essencial para a descompactação correta.
 *
 * @param arquivo Caminho para o arquivo comprimido que será verificado.
 */
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

/**
 * Verifica a integridade entre o arquivo compactado e o arquivo descompactado.
 *
 * A função compara byte a byte o arquivo original (compactado) com o arquivo
 * que foi descompactado para garantir que o processo de descompactação foi realizado corretamente.
 * Ela verifica se ambos os arquivos são idênticos após a descompactação, verificando a integridade
 * dos dados ao longo de todo o arquivo.
 *
 * @param arquivo_compactado Caminho para o arquivo compactado a ser verificado.
 * @param arquivo_descompactado Caminho para o arquivo descompactado a ser comparado.
 */
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

/**
 * Função principal que gerencia o menu de opções e executa operações de compactação, descompactação
 * e verificação de headers de arquivos usando o algoritmo de Huffman.
 *
 * A função exibe um menu com três opções:
 * 1. Compactar um arquivo usando o algoritmo de Huffman.
 * 2. Descompactar um arquivo `.huff` e gerar o arquivo descompactado.
 * 3. Verificar o header de um arquivo compactado `.huff`.
 *
 * Dependendo da escolha do usuário, a função realiza as operações apropriadas, incluindo
 * a verificação de integridade do arquivo descompactado, caso solicitado.
 * 
 * @return Retorna 0 em caso de sucesso ou 1 em caso de erro ou entrada inválida.
 */
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
