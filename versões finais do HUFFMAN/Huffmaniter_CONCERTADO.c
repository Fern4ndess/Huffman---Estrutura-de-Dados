// Inclui a biblioteca padrão de entrada e saída de dados.
// Necessária para operações com arquivos (fopen, fread, fwrite, fclose, printf, etc).
#include <stdio.h>

// Inclui a biblioteca padrão de alocação de memória e controle de processos.
// Necessária para uso de malloc(), free(), exit(), etc.
#include <stdlib.h>

// Inclui biblioteca para manipulação de strings.
// Necessária para funções como memcpy, strcpy, strcat, strlen, etc.
#include <string.h>

// Inclui a biblioteca para configuração de localização regional.
// Usada aqui para definir o idioma e permitir caracteres acentuados no console.
#include <locale.h>

// Define um novo nome para o tipo 'unsigned char' chamado 'BYTE'.
// Isso facilita a leitura do código e reforça a ideia de estar lidando com dados binários.
#define BYTE unsigned char

/**
 * @struct No
 * @brief Estrutura que representa um nó da árvore de Huffman.
 *
 * Cada nó pode ser uma folha (com um caractere real) ou um nó interno (caractere genérico '*').
 */
typedef struct No 
{
    BYTE caractere;             /**< Caractere armazenado no nó (apenas em folhas é válido). */
    int frequencia;             /**< Frequência do caractere no arquivo original. */
    struct No *esquerda;        /**< Ponteiro para o nó filho à esquerda. */
    struct No *direita;         /**< Ponteiro para o nó filho à direita. */
} No;

/**
 * @struct Codigo
 * @brief Representa o código binário gerado para um caractere após a compressão.
 *
 * Usado para armazenar os bits que representam o caminho de um caractere na árvore de Huffman.
 */
typedef struct
{
    int bits;                   /**< Quantidade de bits utilizados no código. */
    BYTE codigo[256];           /**< Vetor contendo os bits do código (máximo 256 bits por caractere). */
} Codigo;

/**
 * @struct Heap
 * @brief Estrutura que representa uma heap mínima para construir a árvore de Huffman.
 *
 * A heap é usada para extrair os dois menores elementos de frequência repetidamente.
 */
typedef struct Heap
{
    int tamanho;                /**< Número de elementos atualmente na heap. */
    No *dados[256];             /**< Vetor de ponteiros para os nós, representando a heap. */
} Heap;

/**
 * @struct Frequencia
 * @brief Armazena a frequência de cada byte do arquivo de entrada.
 */
typedef struct
{
    BYTE byte;                  /**< Byte (caractere) cujo número de ocorrências será contabilizado. */
    int frequencia;             /**< Número de vezes que o byte aparece no arquivo de entrada. */
} Frequencia;

/**
 * @brief Cria e inicializa um novo nó da árvore de Huffman.
 * 
 * @param caractere O caractere armazenado no nó.
 * @param frequencia A frequência do caractere.
 * @param esquerda Ponteiro para o filho esquerdo.
 * @param direita Ponteiro para o filho direito.
 * @return Ponteiro para o novo nó alocado dinamicamente.
 */
No* criar_no(BYTE caractere, int frequencia, No *esquerda, No *direita)
{
    No *novo = (No *) malloc(sizeof(No));  // Aloca memória para o novo nó.
    novo->caractere = caractere;           // Define o caractere do nó.
    novo->frequencia = frequencia;         // Define a frequência do caractere.
    novo->esquerda = esquerda;             // Define o ponteiro para o filho esquerdo.
    novo->direita = direita;               // Define o ponteiro para o filho direito.
    return novo;                           // Retorna o ponteiro para o novo nó.
}

/**
 * @brief Verifica se o nó atual é uma folha (não possui filhos).
 *
 * @param no Ponteiro para o nó que será verificado.
 * @return 1 se o nó for folha, 0 caso contrário.
 */
int eh_folha(No *no)
{
    // Um nó é considerado folha quando seus dois ponteiros filhos são nulos.
    return (no->esquerda == NULL && no->direita == NULL);
}

/**
 * @brief Cria e inicializa uma nova heap vazia.
 * 
 * @return Ponteiro para a heap recém-criada.
 */
Heap* criar_heap()
{
    Heap *heap = (Heap *) malloc(sizeof(Heap)); // Aloca memória para a heap.
    heap->tamanho = 0;                          // Inicializa a heap com tamanho 0 (vazia).
    return heap;                                // Retorna ponteiro para a heap.
}

/**
 * @brief Troca o conteúdo de dois ponteiros de nó.
 * 
 * @param a Ponteiro para o primeiro ponteiro de nó.
 * @param b Ponteiro para o segundo ponteiro de nó.
 */
void trocar(No **a, No **b)
{
    No *temp = *a;  // Salva o valor de 'a' em uma variável temporária.
    *a = *b;        // Atribui o valor de 'b' para 'a'.
    *b = temp;      // Atribui o valor temporário (original de 'a') para 'b'.
}

/**
 * @brief Insere um novo nó na heap mantendo a propriedade de heap mínima.
 *
 * @param heap Ponteiro para a heap onde o nó será inserido.
 * @param no Ponteiro para o nó a ser inserido.
 */
void inserir_heap(Heap *heap, No *no)
{
    int i = heap->tamanho++;   // A posição de inserção é o final da heap (tamanho atual), e o tamanho é incrementado.
    heap->dados[i] = no;       // O nó é colocado no final da heap.

    // Enquanto o nó não estiver na raiz (posição 0) e for menor que seu pai:
    while (i && heap->dados[i]->frequencia < heap->dados[(i - 1) / 2]->frequencia)
    {
        // Troca o nó atual com seu pai.
        trocar(&heap->dados[i], &heap->dados[(i - 1) / 2]);
        i = (i - 1) / 2;       // Atualiza o índice do nó atual para o do pai.
    }
}

/**
 * @brief Remove e retorna o nó com menor frequência da heap (raiz).
 * 
 * @param heap Ponteiro para a heap de onde será removido o nó mínimo.
 * @return Ponteiro para o nó com menor frequência (raiz da heap).
 */
No* remover_min(Heap *heap)
{
    No *minimo = heap->dados[0];          // Armazena o nó com menor frequência (posição 0).
    heap->dados[0] = heap->dados[--heap->tamanho]; // Move o último nó da heap para a raiz e reduz o tamanho.

    int i = 0;                            // Começa na raiz da heap.
    // Enquanto houver pelo menos um filho à esquerda:
    while (i * 2 + 1 < heap->tamanho)
    {
        int menor = i * 2 + 1; // Assume inicialmente que o menor filho é o da esquerda.

        // Se houver filho direito e ele for menor que o esquerdo, atualiza menor.
        if (menor + 1 < heap->tamanho && heap->dados[menor + 1]->frequencia < heap->dados[menor]->frequencia)
            menor++;

        // Se o nó atual já for menor ou igual ao menor filho, termina o ajuste.
        if (heap->dados[i]->frequencia <= heap->dados[menor]->frequencia)
            break;

        // Caso contrário, troca o nó atual com o menor filho e continua o ajuste.
        trocar(&heap->dados[i], &heap->dados[menor]);
        i = menor; // Atualiza o índice do nó atual para o novo índice onde ele foi movido.
    }

    return minimo; // Retorna o nó de menor frequência originalmente removido da raiz.
}

/**

    @brief Conta a frequência de cada byte (0 a 255) no arquivo de entrada.

    @param arquivo Ponteiro para o arquivo de entrada já aberto em modo binário.

    @param tabela Vetor de 256 elementos que irá armazenar a frequência de cada byte.
*/
void contar_frequencias(FILE *arquivo, Frequencia *tabela)
{
    // Inicializa o vetor de frequência: define o valor de cada byte (0 a 255) e zera suas frequências
    for (int i = 0; i < 256; i++)
    {
        tabela[i].byte = (BYTE)i;       // Define o byte (índice convertido para BYTE)
        tabela[i].frequencia = 0;       // Inicializa frequência como 0
    }

    BYTE c;                              // Variável usada para ler byte por byte do arquivo
    // Lê um byte por vez do arquivo até o final
    while (fread(&c, sizeof(BYTE), 1, arquivo))
    {
        tabela[c].frequencia++;         // Incrementa a frequência correspondente ao byte lido
    }
}

/**

    @brief Constrói a árvore de Huffman usando uma heap mínima com base nas frequências.

    @param tabela Vetor de 256 estruturas Frequencia com dados de cada byte.

    @return Ponteiro para a raiz da árvore de Huffman construída.
*/
No *construir_arvore(Frequencia *tabela)
{
    Heap *heap = criar_heap();          // Cria uma heap vazia para armazenar os nós

    // Insere todos os bytes com frequência > 0 na heap como nós folha
    for (int i = 0; i < 256; i++)
    {
        if (tabela[i].frequencia > 0)
        {
            // Cria um nó folha com caractere, frequência e sem filhos, e insere na heap
            inserir_heap(heap, criar_no(tabela[i].byte, tabela[i].frequencia, NULL, NULL));
        }
    }

    // Enquanto houver mais de um nó na heap, remove os dois menores e une
    while (heap->tamanho > 1)
    {
        No *esq = remover_min(heap);    // Remove o nó com menor frequência (filho esquerdo)
        No *dir = remover_min(heap);    // Remove o próximo menor (filho direito)

        // Cria um novo nó interno com caractere '*' e frequência somada
        inserir_heap(heap, criar_no('*', esq->frequencia + dir->frequencia, esq, dir));
    }

    return remover_min(heap);           // Ao final, retorna o único nó restante: a raiz da árvore
}

/**
    @brief Gera os códigos binários de Huffman para cada caractere a partir da árvore.

    @param raiz Ponteiro para a raiz da árvore de Huffman.

    @param tabela Vetor de 256 códigos, um para cada caractere.

    @param codigo Vetor temporário que armazena os bits do caminho atual.

    @param nivel Nível atual na árvore, que indica a profundidade (e número de bits do código).
*/
void gerar_codigos(No *raiz, Codigo *tabela, BYTE *codigo, int nivel)
{
    if (raiz == NULL)                   // Caso base: se o nó for nulo, retorna
        return;

    if (eh_folha(raiz))                 // Se chegou a um nó folha (caractere válido)
    {
        tabela[raiz->caractere].bits = nivel;                // Armazena o número de bits do código
        memcpy(tabela[raiz->caractere].codigo, codigo, nivel); // Copia os bits do caminho para a tabela
        return;                                              // Termina a recursão
    }

    // Caminho à esquerda = bit 0
    codigo[nivel] = 0;                 
    gerar_codigos(raiz->esquerda, tabela, codigo, nivel + 1); // Chama recursivamente para o filho esquerdo

    // Caminho à direita = bit 1
    codigo[nivel] = 1;
    gerar_codigos(raiz->direita, tabela, codigo, nivel + 1);  // Chama recursivamente para o filho direito
}


/**
    @brief Serializa (escreve) a árvore de Huffman no arquivo de saída.

    @param raiz Ponteiro para a raiz da árvore.

    @param out Arquivo de saída já aberto em modo binário.

    @param tamanho Ponteiro para contador que armazena o tamanho da árvore em bytes.
*/
void escrever_arvore(No *raiz, FILE *out, int *tamanho)
{
    if (raiz == NULL)                  // Caso base: se o nó for nulo, retorna
        return;

    if (eh_folha(raiz))                // Se for folha, escreve o caractere diretamente
    {
        BYTE c = raiz->caractere;      // Obtém o caractere armazenado

        if (c == '*' || c == '\\')     // Se for caractere especial ('*' ou '\'), precisa escapar
        {
            BYTE barra = '\\';         // Usa caractere de escape
            fwrite(&barra, sizeof(BYTE), 1, out);  // Escreve escape no arquivo
            (*tamanho)++;              // Atualiza o contador de tamanho
        }

        fwrite(&c, sizeof(BYTE), 1, out);  // Escreve o caractere em si
        (*tamanho)++;                      // Incrementa o tamanho da árvore
        return;
    }

    BYTE estrela = '*';               // Nó interno é representado por '*'
    fwrite(&estrela, sizeof(BYTE), 1, out); // Escreve '*' no arquivo
    (*tamanho)++;                         // Atualiza o tamanho da árvore

    escrever_arvore(raiz->esquerda, out, tamanho); // Recursivamente escreve a subárvore esquerda
    escrever_arvore(raiz->direita, out, tamanho);  // Recursivamente escreve a subárvore direita
}


/**
    @brief Escreve os dois primeiros bytes (16 bits) do arquivo compactado (header).

    @param out Ponteiro para o arquivo de saída.

    @param trash_bits Número de bits de lixo no último byte.

    @param tree_size Tamanho da árvore serializada (em bytes).
*/
void escrever_header(FILE *out, int trash_bits, int tree_size)
{
    // Cria o valor de 16 bits com os 3 bits de lixo no início + 13 bits de tamanho da árvore
    unsigned short header = (trash_bits << 13) | tree_size;

    BYTE byte1 = header >> 8;         // Extrai os 8 bits mais significativos (MSB)
    BYTE byte2 = header & 0xFF;       // Extrai os 8 bits menos significativos (LSB)

    fwrite(&byte1, sizeof(BYTE), 1, out); // Escreve primeiro byte (MSB)
    fwrite(&byte2, sizeof(BYTE), 1, out); // Escreve segundo byte (LSB)
}

/**
    @brief Lê os dois primeiros bytes do arquivo compactado e extrai os metadados.

    @param in Ponteiro para o arquivo de entrada.

    @param trash_bits Ponteiro onde será armazenado o número de bits de lixo.

    @param tree_size Ponteiro onde será armazenado o tamanho da árvore serializada.
*/
void ler_header(FILE *in, int *trash_bits, unsigned short *tree_size)
{
    BYTE byte1, byte2;                    // Variáveis para armazenar os dois bytes do header

    fread(&byte1, sizeof(BYTE), 1, in);   // Lê o primeiro byte do cabeçalho
    fread(&byte2, sizeof(BYTE), 1, in);   // Lê o segundo byte

    unsigned short header = (byte1 << 8) | byte2; // Junta os dois bytes em um inteiro de 16 bits

    *trash_bits = header >> 13;                   // Extrai os 3 bits mais significativos (bits de lixo)
    *tree_size = header & 0x1FFF;                 // Extrai os 13 bits restantes (máscara 13 bits) => tamanho da árvore
}


// Função que reconstrói a árvore de Huffman a partir da sua representação serializada no arquivo
No* reconstruir_arvore(FILE *in, int *pos) 
{
    BYTE c; // Variável que armazenará o caractere lido do arquivo

    fread(&c, sizeof(BYTE), 1, in); // Lê 1 byte do arquivo (pode ser '*', '\\' ou um caractere literal)
    (*pos)--; // Decrementa a quantidade restante de bytes da árvore (controla o tamanho da árvore lida)

    if (c == '*') // Se o caractere lido for '*', significa que é um nó interno da árvore
    {
        No *esq = reconstruir_arvore(in, pos); // Recursivamente reconstrói a subárvore esquerda
        No *dir = reconstruir_arvore(in, pos); // Recursivamente reconstrói a subárvore direita
        return criar_no('*', 0, esq, dir); // Cria um novo nó interno com os filhos esquerdo e direito
    }
    else if (c == '\\') // Se o caractere lido for '\\', significa que o próximo caractere foi escapado
    {
        fread(&c, sizeof(BYTE), 1, in); // Lê o caractere real que foi escapado
        (*pos)--; // Decrementa o contador pois foi lido mais um byte
    }

    return criar_no(c, 0, NULL, NULL); // Retorna um nó folha com o caractere lido
}

// Função que compacta o arquivo de entrada usando o algoritmo de Huffman
void compactar_arquivo(const char *entrada, const char *saida)
{
    FILE *in = fopen(entrada, "rb"); // Abre o arquivo de entrada no modo leitura binária
    if (!in) // Verifica se houve erro na abertura do arquivo
    {
        printf("Erro ao abrir arquivo de entrada\n"); // Exibe mensagem de erro
        return; // Interrompe a execução da função
    }

    Frequencia tabela_frequencias[256]; // Tabela que armazenará a frequência de cada byte (0 a 255)
    contar_frequencias(in, tabela_frequencias); // Conta a frequência de cada byte presente no arquivo

    No *raiz = construir_arvore(tabela_frequencias); // Constrói a árvore de Huffman a partir das frequências

    Codigo tabela[256] = {0}; // Inicializa a tabela de códigos Huffman com zeros
    BYTE codigo[256]; // Vetor auxiliar usado para armazenar o caminho do código de Huffman
    gerar_codigos(raiz, tabela, codigo, 0); // Gera os códigos de Huffman para todos os caracteres

    FILE *out = fopen(saida, "wb"); // Abre o arquivo de saída no modo escrita binária
    fseek(out, 2, SEEK_SET); // Pula os dois primeiros bytes (serão usados depois para o header)

    rewind(in); // Retorna o ponteiro do arquivo de entrada para o início
    fseek(out, 2, SEEK_SET); // Garante novamente que vamos pular os dois bytes do header

    int tree_size = 0; // Armazena o tamanho da árvore que será escrita
    escrever_arvore(raiz, out, &tree_size); // Escreve a árvore de Huffman no arquivo de saída

    BYTE buffer = 0; // Armazena temporariamente os bits até formar um byte completo
    int bits_usados = 0; // Contador de quantos bits já foram inseridos no buffer
    BYTE c; // Variável para armazenar cada caractere lido do arquivo de entrada

    while (fread(&c, sizeof(BYTE), 1, in)) // Lê byte a byte do arquivo de entrada
    {
        for (int i = 0; i < tabela[c].bits; i++) // Para cada bit do código Huffman do caractere
        {
            buffer <<= 1; // Desloca o buffer para a esquerda para abrir espaço para o novo bit
            if (tabela[c].codigo[i]) // Se o bit atual é 1
                buffer |= 1; // Adiciona 1 no bit menos significativo do buffer

            bits_usados++; // Incrementa a contagem de bits inseridos no buffer

            if (bits_usados == 8) // Se o buffer está cheio (8 bits)
            {
                fwrite(&buffer, sizeof(BYTE), 1, out); // Escreve o byte no arquivo de saída
                bits_usados = 0; // Reseta o número de bits usados
                buffer = 0; // Limpa o buffer
            }
        }
    }

    if (bits_usados > 0) // Se sobrou bits no buffer (último byte incompleto)
    {
        buffer <<= (8 - bits_usados); // Preenche os bits restantes com 0 (lixo)
        fwrite(&buffer, sizeof(BYTE), 1, out); // Escreve o último byte no arquivo
    }

    int trash_bits = bits_usados ? 8 - bits_usados : 0; // Calcula quantos bits foram lixo no último byte
    rewind(out); // Volta ao início do arquivo para escrever o header
    escrever_header(out, trash_bits, tree_size); // Escreve os dois primeiros bytes (header)

    fclose(in); // Fecha o arquivo de entrada
    fclose(out); // Fecha o arquivo de saída
    printf("Arquivo compactado com sucesso!\n"); // Mensagem de sucesso
}

// Função que descompacta um arquivo .huff gerado com o algoritmo de Huffman
void descompactar_arquivo(const char *entrada, const char *saida)
{
    FILE *in = fopen(entrada, "rb"); // Abre o arquivo compactado para leitura binária
    if (!in) // Verifica se a abertura falhou
    {
        printf("Erro ao abrir arquivo de entrada\n"); // Exibe mensagem de erro
        return; // Interrompe a execução
    }

    int trash_bits; // Quantidade de bits de lixo no final do arquivo
    unsigned short tree_size; // Tamanho (em bytes) da árvore de Huffman serializada
    ler_header(in, &trash_bits, &tree_size); // Lê os dois primeiros bytes (header) do arquivo

    int pos = tree_size; // Inicializa a variável de controle de posição com o tamanho da árvore
    No *raiz = reconstruir_arvore(in, &pos); // Reconstrói a árvore de Huffman a partir da leitura

    FILE *out = fopen(saida, "wb"); // Abre o arquivo descompactado para escrita binária
    No *atual = raiz; // Começa a navegação pela árvore a partir da raiz

    BYTE c; // Armazena cada byte lido
    long total_bytes = ftell(in); // Salva a posição atual do ponteiro de leitura (após a árvore)
    fseek(in, 0, SEEK_END); // Vai para o final do arquivo
    long tamanho_total = ftell(in) - total_bytes; // Calcula quantos bytes ainda faltam ler (parte codificada)
    fseek(in, total_bytes, SEEK_SET); // Volta para o início da parte codificada

    for (long i = 0; i < tamanho_total; i++) // Percorre todos os bytes compactados
    {
        fread(&c, sizeof(BYTE), 1, in); // Lê um byte do arquivo compactado
        int limite_bits = 8; // Inicialmente, todos os 8 bits do byte são considerados úteis

        if (i == tamanho_total - 1) // Se for o último byte, considera os bits de lixo
            limite_bits = 8 - trash_bits;

        for (int j = 7; j >= 8 - limite_bits; j--) // Percorre os bits úteis do byte, do mais para o menos significativo
        {
            int bit = (c >> j) & 1; // Extrai o bit j do byte c
            atual = bit ? atual->direita : atual->esquerda; // Caminha na árvore: 1 para direita, 0 para esquerda

            if (eh_folha(atual)) // Se chegou em uma folha (caractere completo)
            {
                fwrite(&atual->caractere, sizeof(BYTE), 1, out); // Escreve o caractere original no arquivo de saída
                atual = raiz; // Volta para o início da árvore para decodificar o próximo caractere
            }
        }
    }

    fclose(in); // Fecha o arquivo compactado
    fclose(out); // Fecha o arquivo descompactado
    printf("Arquivo descompactado com sucesso!\n"); // Mensagem de sucesso
}



// Função principal do programa Huffman Compressor
int main()
{
    // Define a localidade para "Portuguese" para permitir uso de acentos e caracteres especiais na saída do console
    setlocale(LC_ALL, "Portuguese");

    int opcao; // Variável que armazenará a opção escolhida pelo usuário no menu
    char nome_arquivo[256] = {0};  // Vetor para armazenar o nome do arquivo digitado pelo usuário
    char nome_saida[256] = {0};    // Vetor para armazenar o nome do arquivo de saída
    char nome_original[256] = {0}; // Vetor auxiliar que mantém o nome original do arquivo antes de ser modificado

    // Imprime o menu inicial do programa
    printf("Seja bem-vindo ao Huffman!\n");
    printf("1. Compactar arquivo\n");
    printf("2. Descompactar arquivo\n");

    // Solicita ao usuário que selecione uma das opções
    printf("Escolha: ");
    if (scanf("%d", &opcao) != 1) // Lê a opção do usuário e verifica se a entrada foi válida (um número)
    {
        printf("Entrada inválida\n"); // Se o scanf falhou (usuário digitou algo não numérico)
        return 1; // Encerra o programa com código de erro
    }

    getchar(); // Consome o caractere '\n' deixado no buffer pelo scanf

    // Caso o usuário tenha escolhido a opção 1 - Compactar
    if (opcao == 1)
    {
        printf("Arquivo a compactar: ");
        fgets(nome_arquivo, sizeof(nome_arquivo), stdin); // Lê o nome do arquivo digitado (permite espaços)

        // Remove o caractere de nova linha '\n' que fica no final após o uso de fgets
        nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';

        strcpy(nome_original, nome_arquivo); // Salva o nome original em uma variável auxiliar

        strcat(nome_arquivo, ".huff"); // Adiciona a extensão ".huff" ao nome do arquivo de saída

        compactar_arquivo(nome_original, nome_arquivo); // Chama a função de compactação
    }
    // Caso o usuário tenha escolhido a opção 2 - Descompactar
    else if (opcao == 2)
    {
        printf("Arquivo .huff a descompactar: ");
        fgets(nome_arquivo, sizeof(nome_arquivo), stdin); // Lê o nome do arquivo a descompactar

        // Remove o caractere de nova linha '\n'
        nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';

        // Verifica se o nome digitado realmente termina com ".huff"
        if (!strstr(nome_arquivo, ".huff"))
        {
            printf("Deve ser um arquivo .huff\n"); // Se não for, exibe mensagem de erro
            return 1; // Encerra o programa com erro
        }

        // Copia o nome do arquivo sem a extensão ".huff"
        strncpy(nome_saida, nome_arquivo, strlen(nome_arquivo) - 5);
        nome_saida[strlen(nome_arquivo) - 5] = '\0'; // Termina a string manualmente

        strcat(nome_saida, ".dehuff"); // Adiciona a extensão ".dehuff" ao nome do arquivo de saída

        descompactar_arquivo(nome_arquivo, nome_saida); // Chama a função de descompactação
    }
    else
    {
        printf("Opção inválida!\n"); // Caso a opção não seja 1 nem 2
        return 1; // Retorna código de erro
    }

    return 0; // Retorna 0 para indicar que o programa terminou com sucesso
}
