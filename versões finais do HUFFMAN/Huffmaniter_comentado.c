#include <stdio.h>   // Para funções de entrada/saída (fopen, fread, etc.)
#include <stdlib.h>  // Para alocação de memória (malloc, free) e outras funções
#include <string.h>  // Para manipulação de strings (memcpy, strcpy, etc.)
#include <locale.h>  // Para configuração de localização (setlocale)


#define BYTE unsigned char  // Define um tipo BYTE como unsigned char (1 bit)

// Estrutura para representar um nó da árvore de Huffman
typedef struct No {
    BYTE caractere;       // O caractere armazenado no nó (para folhas)
    int frequencia;       // Frequência do caractere
    struct No *esquerda;  // Ponteiro para o filho esquerdo
    struct No *direita;   // Ponteiro para o filho direito
} No;

// Estrutura para armazenar códigos de Huffman
typedef struct {
    BYTE byte;        // guarda o caractere original e também qual caractere o código representa
    int bits;         // quantos bits existem no código
    BYTE codigo[256]; // O código de Huffman (sequência de bits)
} Codigo;

// Estrutura para a heap mínima usada na construção da árvore
typedef struct Heap {
    int tamanho;      // Tamanho atual da heap
    No *dados[256];   // Array de ponteiros para nós
} Heap;

// Estrutura para armazenar frequência de bytes (não usada diretamente no código principal)
typedef struct {
    BYTE byte;
    int frequencia;
} Frequencia;

// Outra estrutura para frequência (também não usada diretamente)
typedef struct {
    BYTE byte;
    int frequencia;
} Item;

// Funções da árvore de Huffman -------------------------------------------------

// Cria um novo nó da árvore de Huffman
No* criar_no(BYTE caractere, int frequencia, No *esquerda, No *direita)
{
    No *novo = (No *) malloc(sizeof(No));  // Aloca memória para o novo nó
    novo->caractere = caractere;           // Atribui o caractere
    novo->frequencia = frequencia;         // Atribui a frequência
    novo->esquerda = esquerda;             // Define o filho esquerdo
    novo->direita = direita;               // Define o filho direito
    return novo;                           // Retorna o novo nó
}

// Verifica se um nó é folha (não tem filhos)
int eh_folha(No *no)
{
    return (no->esquerda == NULL && no->direita == NULL);  // Retorna 1 se for folha, 0 caso contrário
}

// Funções da heap -------------------------------------------------------------

// Cria uma nova heap vazia
Heap* criar_heap()
{
    Heap *heap = (Heap *) malloc(sizeof(Heap));  // Aloca memória para a heap
    heap->tamanho = 0;                          // Inicializa o tamanho como 0
    return heap;                                // Retorna a nova heap
}

// Troca dois ponteiros de nós
void trocar(No **a, No **b)
{
    No *temp = *a;  // Armazena o valor de a temporariamente
    *a = *b;        // Atribui o valor de b a a
    *b = temp;      // Atribui o valor temporário a b
}

// Insere um nó na heap mantendo a propriedade de heap mínima
void inserir_heap(Heap *heap, No *no)
{
    int i = heap->tamanho++;  // Incrementa o tamanho e obtém o índice do novo elemento
    heap->dados[i] = no;      // Coloca o novo nó na última posição

    // Ajusta a heap subindo o novo elemento se necessário
    while (i && heap->dados[i]->frequencia < heap->dados[(i - 1) / 2]->frequencia)
    {
        trocar(&heap->dados[i], &heap->dados[(i - 1) / 2]);  // Troca com o pai
        i = (i - 1) / 2;                                    // Atualiza o índice para o pai
    }
}

// Remove e retorna o nó com menor frequência (raiz da heap mínima)
No* remover_min(Heap *heap)
{
    No *minimo = heap->dados[0];            // Armazena o nó mínimo (raiz)
    heap->dados[0] = heap->dados[--heap->tamanho];  // Move o último elemento para a raiz

    int i = 0;
    // Ajusta a heap descendo o elemento raiz se necessário
    while (i * 2 + 1 < heap->tamanho)
    {
        int menor = i * 2 + 1;  // Índice do filho esquerdo
        // Se o filho direito existe e é menor que o esquerdo
        if (menor + 1 < heap->tamanho && heap->dados[menor + 1]->frequencia < heap->dados[menor]->frequencia)
            menor++;  // Usa o filho direito

        // Se a propriedade da heap já está satisfeita, para
        if (heap->dados[i]->frequencia <= heap->dados[menor]->frequencia)
            break;

        trocar(&heap->dados[i], &heap->dados[menor]);  // Troca com o filho menor
        i = menor;                                     // Continua a verificação no nível inferior
    }

    return minimo;  // Retorna o nó removido
}

// Conta a frequência de cada byte no arquivo
void contar_frequencias(FILE *arquivo, int *frequencias)
{
    BYTE c;
    // Lê cada byte do arquivo e incrementa sua frequência
    while (fread(&c, sizeof(BYTE), 1, arquivo))
    {
        frequencias[c]++;  // Incrementa a contagem para o byte lido
    }
}

// Constrói a árvore de Huffman a partir das frequências
No* construir_arvore(int *frequencias)
{
    Heap *heap = criar_heap();  // Cria uma heap vazia
    
    // Para cada byte possível (0-255)
    for (int i = 0; i < 256; i++)
    {
        // Se o byte aparece no arquivo
        if (frequencias[i])
        {
            // Cria um nó folha e insere na heap
            inserir_heap(heap, criar_no((BYTE)i, frequencias[i], NULL, NULL));
        }
    }

    // Combina os nós até restar apenas um (a raiz da árvore)
    while (heap->tamanho > 1)
    {
        No *esq = remover_min(heap);  // Remove o nó com menor frequência
        No *dir = remover_min(heap);  // Remove o próximo nó com menor frequência
        // Cria um novo nó interno combinando os dois nós
        inserir_heap(heap, criar_no('*', esq->frequencia + dir->frequencia, esq, dir));
    }

    return remover_min(heap);  // Retorna a raiz da árvore
}

// Gera os códigos de Huffman percorrendo a árvore
// Pega os dados da árvore e transforma em binário
void gerar_codigos(No *raiz, Codigo *tabela, BYTE *codigo, int nivel)
{
    if (raiz == NULL)  // Se o nó é nulo, retorna
        return;

    // Se é uma folha, armazena o código na tabela
    if (eh_folha(raiz))
    {
        tabela[raiz->caractere].bits = nivel;  // Armazena o tamanho do código
        // Copia o código gerado para a tabela
        memcpy(tabela[raiz->caractere].codigo, codigo, nivel);
        return;
    }

    // Percorre a esquerda (bit 0)
    codigo[nivel] = 0;
    gerar_codigos(raiz->esquerda, tabela, codigo, nivel + 1);

    // Percorre a direita (bit 1)
    codigo[nivel] = 1;
    gerar_codigos(raiz->direita, tabela, codigo, nivel + 1);
}

// Escreve a árvore no arquivo compactado (em pré-ordem)
//
void escrever_arvore(No *raiz, FILE *out, int *tamanho)
{
    if (raiz == NULL)  // Se o nó é nulo, retorna
        return;

    // Se é uma folha
    if (eh_folha(raiz))
    {
        BYTE c = raiz->caractere;
        // Se o caractere é '*' ou '\', precisa ser escapado
        if (c == '*' || c == '\\')
        {
            BYTE barra = '\\';
            fwrite(&barra, sizeof(BYTE), 1, out);  // Escreve o escape
            (*tamanho)++;  // Incrementa o tamanho da árvore
        }
        fwrite(&c, sizeof(BYTE), 1, out);  // Escreve o caractere
        (*tamanho)++;  // Incrementa o tamanho da árvore
        return;
    }

    // Escreve um nó interno (marcado com '*')
    BYTE estrela = '*';
    fwrite(&estrela, sizeof(BYTE), 1, out);
    (*tamanho)++;
    // Percorre os filhos
    escrever_arvore(raiz->esquerda, out, tamanho);
    escrever_arvore(raiz->direita, out, tamanho);
}

// Escreve o cabeçalho do arquivo compactado
void escrever_header(FILE *out, int trash_bits, int tree_size)
{
    // Formato do header: 3 bits para lixo + 13 bits para tamanho da árvore
    unsigned short header = (trash_bits << 13) | tree_size;
    // Divide em 2 bytes
    BYTE byte1 = header >> 8;
    BYTE byte2 = header & 0xFF;
    // Escreve os bytes no arquivo
    fwrite(&byte1, sizeof(BYTE), 1, out);
    fwrite(&byte2, sizeof(BYTE), 1, out);
}

// Lê o cabeçalho do arquivo compactado
void ler_header(FILE *in, int *trash_bits, unsigned short *tree_size)
{
    BYTE byte1, byte2;
    // Lê os 2 bytes do header
    fread(&byte1, sizeof(BYTE), 1, in);
    fread(&byte2, sizeof(BYTE), 1, in);
    // Combina os bytes em um short
    unsigned short header = (byte1 << 8) | byte2;
    // Extrai os bits de lixo (3 primeiros bits)
    *trash_bits = header >> 13;
    // Extrai o tamanho da árvore (13 bits restantes)
    *tree_size = header & 0x1FFF;
}

// Reconstrói a árvore de Huffman a partir do arquivo
No* reconstruir_arvore(FILE *in, int *pos)
{
    BYTE c;
    fread(&c, sizeof(BYTE), 1, in);  // Lê um byte
    (*pos)--;  // Decrementa o contador de bytes restantes

    // Se for um nó interno
    if (c == '*')
    {
        // Reconstrói recursivamente os filhos
        No *esq = reconstruir_arvore(in, pos);
        No *dir = reconstruir_arvore(in, pos);
        return criar_no('*', 0, esq, dir);  // Cria o nó interno
    }
    else if (c == '\\')  // Se for um caractere escapado
    {
        fread(&c, sizeof(BYTE), 1, in);  // Lê o próximo byte
        (*pos)--;  // Decrementa o contador
    }

    // Cria um nó folha com o caractere lido
    return criar_no(c, 0, NULL, NULL);
}

// Função principal para compactar um arquivo
void compactar_arquivo(const char *entrada, const char *saida)
{
    FILE *in = fopen(entrada, "rb");  // Abre o arquivo de entrada
    if (!in)
    {
        printf("Erro ao abrir arquivo de entrada\n");
        return;
    }

    int frequencias[256] = {0};  // Inicializa o array de frequências
    contar_frequencias(in, frequencias);  // Conta as frequências
    rewind(in);  // Volta ao início do arquivo

    // Constrói a árvore de Huffman
    No *raiz = construir_arvore(frequencias);
    Codigo tabela[256] = {0};  // Tabela de códigos
    BYTE codigo[256];          // Buffer para construção de códigos
    gerar_codigos(raiz, tabela, codigo, 0);  // Gera os códigos

    FILE *out = fopen(saida, "wb");  // Abre o arquivo de saída
    fseek(out, 2, SEEK_SET);  // Pula o espaço do header (será escrito depois)

    int tree_size = 0;  // Tamanho da árvore serializada
    escrever_arvore(raiz, out, &tree_size);  // Escreve a árvore

    BYTE buffer = 0;    // Buffer para acumular bits
    int bits_usados = 0;  // Contador de bits no buffer
    BYTE c;
    // Processa cada byte do arquivo original
    while (fread(&c, sizeof(BYTE), 1, in))
    {
        // Para cada bit do código Huffman do byte atual
        for (int i = 0; i < tabela[c].bits; i++)
        {
            buffer <<= 1;  // Desloca o buffer para esquerda
            if (tabela[c].codigo[i])  // Se o bit for 1
                buffer |= 1;          // Adiciona 1 ao buffer
            bits_usados++;

            // Se o buffer está cheio (8 bits), escreve no arquivo
            if (bits_usados == 8)
            {
                fwrite(&buffer, sizeof(BYTE), 1, out);
                bits_usados = 0;
                buffer = 0;
            }
        }
    }

    // Se sobrou bits no buffer, completa com zeros e escreve
    if (bits_usados > 0) {
        buffer <<= (8 - bits_usados);
        fwrite(&buffer, sizeof(BYTE), 1, out);
    }

    // Calcula os bits de lixo (bits não utilizados no último byte)
    int trash_bits = bits_usados ? 8 - bits_usados : 0;
    rewind(out);  // Volta ao início para escrever o header
    escrever_header(out, trash_bits, tree_size);  // Escreve o header

    fclose(in);  // Fecha os arquivos
    fclose(out);
    printf("Arquivo compactado com sucesso!\n");
}

// Função principal para descompactar um arquivo
void descompactar_arquivo(const char *entrada, const char *saida)
{
    FILE *in = fopen(entrada, "rb");  // Abre o arquivo compactado
    if (!in)
    {
        printf("Erro ao abrir arquivo de entrada\n");
        return;
    }

    int trash_bits;
    unsigned short tree_size;
    ler_header(in, &trash_bits, &tree_size);  // Lê o header

    int pos = tree_size;
    // Reconstrói a árvore de Huffman
    No *raiz = reconstruir_arvore(in, &pos);

    FILE *out = fopen(saida, "wb");  // Abre o arquivo de saída
    No *atual = raiz;  // Ponteiro para percorrer a árvore
    BYTE c;
    long total_bytes = ftell(in);  // Posição atual no arquivo (após a árvore)
    fseek(in, 0, SEEK_END);
    long tamanho_total = ftell(in) - total_bytes;  // Tamanho dos dados codificados
    fseek(in, total_bytes, SEEK_SET);

    // Processa cada byte dos dados codificados
    for (long i = 0; i < tamanho_total; i++)
    {
        fread(&c, sizeof(BYTE), 1, in);  // Lê um byte
        // Processa cada bit do byte
        for (int j = 7; j >= 0; j--)
        {
            int bit = (c >> j) & 1;  // Extrai o bit
            // Percorre a árvore: 0 = esquerda, 1 = direita
            atual = bit ? atual->direita : atual->esquerda;

            // Se chegou a uma folha
            if (eh_folha(atual))
            {
                fwrite(&atual->caractere, sizeof(BYTE), 1, out);  // Escreve o caractere
                atual = raiz;  // Volta para a raiz
            }

            // Se é o último byte e últimos bits de lixo, para
            if (i == tamanho_total - 1 && j == trash_bits)
                break;
        }
    }

    fclose(in);  // Fecha os arquivos
    fclose(out);
    printf("Arquivo descompactado com sucesso!\n");
}

// Função para verificar o header de um arquivo compactado
void verificar_header(const char *arquivo)
{
    FILE *in = fopen(arquivo, "rb");  // Abre o arquivo
    if (!in)
    {
        perror("Erro ao abrir arquivo");
        return;
    }

    int trash_bits;
    unsigned short tree_size;
    ler_header(in, &trash_bits, &tree_size);  // Lê o header

    // Exibe as informações do header
    printf("Header do arquivo %s:\n", arquivo);
    printf("- Bits de lixo: %d\n", trash_bits);
    printf("- Tamanho da árvore: %hu bytes\n", tree_size);

    fclose(in);  // Fecha o arquivo
}

// Função para verificar se a descompactação foi correta
void verificar_integridade(const char *arquivo_compactado, const char *arquivo_descompactado)
{
    FILE *f1 = fopen(arquivo_compactado, "rb");  // Abre o arquivo compactado
    FILE *f2 = fopen(arquivo_descompactado, "rb");  // Abre o arquivo descompactado

    if (!f1 || !f2)
    {
        printf("Erro ao abrir arquivos para verificação\n");
        if (f1) fclose(f1);
        if (f2) fclose(f2);
        return;
    }

    int trash_bits;
    unsigned short tree_size;
    ler_header(f1, &trash_bits, &tree_size);  // Lê o header
    fseek(f1, 2 + tree_size, SEEK_SET);  // Pula para os dados codificados

    int erro = 0;
    long offset = 0;
    int c1, c2;

    // Compara byte a byte
    while ((c1 = fgetc(f1)) != EOF && (c2 = fgetc(f2)) != EOF)
    {
        if (c1 != c2)  // Se encontrar diferença
        {
            erro = 1;
            break;
        }
        offset++;
    }

    // Verifica se um arquivo terminou antes do outro
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

    fclose(f1);  // Fecha os arquivos
    fclose(f2);
}

// Função principal (menu do programa)
int main()
{
    setlocale(LC_ALL, "Portuguese");  // Configura localização para português

    int opcao;
    char nome_arquivo[256] = {0};    // Buffer para nome do arquivo
    char nome_saida[256] = {0};      // Buffer para nome de saída
    char nome_original[256] = {0};   // Buffer para nome original

    printf("Huffman File Compressor\n");
    printf("1. Compactar arquivo\n");
    printf("2. Descompactar arquivo\n");
    printf("3. Verificar header\n");
    printf("Escolha: ");
    if (scanf("%d", &opcao) != 1)  // Lê a opção do usuário
    {
        printf("Entrada inválida\n");
        return 1;
    }
    getchar();  // Limpa o buffer do teclado

    if (opcao == 1)  // Compactação
    {
        printf("Arquivo a compactar: ");
        fgets(nome_arquivo, sizeof(nome_arquivo), stdin);  // Lê o nome do arquivo
        nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';  // Remove o \n
        strcpy(nome_original, nome_arquivo);  // Salva o nome original
        strcat(nome_arquivo, ".huff");  // Adiciona extensão .huff
        compactar_arquivo(nome_original, nome_arquivo);  // Chama a função de compactação
    }
    else if (opcao == 2)  // Descompactação
    {
        printf("Arquivo .huff a descompactar: ");
        fgets(nome_arquivo, sizeof(nome_arquivo), stdin);
        nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';

        // Verifica se tem a extensão .huff
        if (!strstr(nome_arquivo, ".huff"))
        {
            printf("Deve ser um arquivo .huff\n");
            return 1;
        }

        // Cria o nome de saída removendo .huff e adicionando .dehuff
        strncpy(nome_saida, nome_arquivo, strlen(nome_arquivo) - 5);
        nome_saida[strlen(nome_arquivo) - 5] = '\0';
        strcat(nome_saida, ".dehuff");

        descompactar_arquivo(nome_arquivo, nome_saida);  // Chama a função de descompactação

        // Pergunta se quer verificar a integridade
        printf("\nDeseja verificar a integridade? (s/n): ");
        char resposta = getchar();
        if (resposta == 's' || resposta == 'S')
        {
            verificar_integridade(nome_arquivo, nome_saida);
        }
    }
    else if (opcao == 3)  // Verificação de header
    {
        printf("Arquivo .huff para verificar header: ");
        fgets(nome_arquivo, sizeof(nome_arquivo), stdin);
        nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';
        verificar_header(nome_arquivo);  // Chama a função de verificação
    }
    else  // Opção inválida
    {
        printf("Opção inválida!\n");
        return 1;
    }

    return 0;  // Fim do programa
}