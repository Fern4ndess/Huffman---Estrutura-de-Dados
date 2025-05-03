#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <locale.h>

#define TAM_ASCII 256
#define BUFFER_SIZE 65536  // 64KB para melhor performance de I/O

typedef struct {
    unsigned char trash : 3;
    unsigned short tree_size : 13;
} HuffmanHeader;

typedef struct NoHuffman {
    unsigned char caractere;
    unsigned long frequencia;  // Alterado para long para suportar arquivos grandes
    struct NoHuffman *esquerda, *direita;
} NoHuffman;

typedef struct NoLista {
    void *dados;
    struct NoLista *prox;
} NoLista;

// Estrutura otimizada para escrita de bits
typedef struct {
    unsigned char buffer[BUFFER_SIZE];
    unsigned char byte_atual;
    unsigned int pos_buffer;
    unsigned int bits_preenchidos;
} BitStream;

// Variáveis globais otimizadas
unsigned char* tabela_codigos[TAM_ASCII];
unsigned char tamanho_codigos[TAM_ASCII];  // Armazena tamanhos dos códigos

// ================= FUNÇÕES OTIMIZADAS =================

void init_bit_stream(BitStream *bs) {
    bs->pos_buffer = 0;
    bs->byte_atual = 0;
    bs->bits_preenchidos = 0;
    memset(bs->buffer, 0, BUFFER_SIZE);
}

void write_bit(BitStream *bs, int bit, FILE *out) {
    bs->byte_atual = (bs->byte_atual << 1) | (bit & 1);
    bs->bits_preenchidos++;
    
    if (bs->bits_preenchidos == 8) {
        bs->buffer[bs->pos_buffer++] = bs->byte_atual;
        if (bs->pos_buffer == BUFFER_SIZE) {
            fwrite(bs->buffer, 1, BUFFER_SIZE, out);
            bs->pos_buffer = 0;
        }
        bs->byte_atual = 0;
        bs->bits_preenchidos = 0;
    }
}

void flush_bit_stream(BitStream *bs, FILE *out) {
    if (bs->bits_preenchidos > 0) {
        bs->byte_atual <<= (8 - bs->bits_preenchidos);
        bs->buffer[bs->pos_buffer++] = bs->byte_atual;
    }
    if (bs->pos_buffer > 0) {
        fwrite(bs->buffer, 1, bs->pos_buffer, out);
    }
}

NoHuffman* criar_no(unsigned char caractere, unsigned long frequencia) {
    NoHuffman* novo = (NoHuffman*)malloc(sizeof(NoHuffman));
    if (!novo) return NULL;
    
    novo->caractere = caractere;
    novo->frequencia = frequencia;
    novo->esquerda = novo->direita = NULL;
    return novo;
}

// Função otimizada para construção da árvore
NoHuffman* construir_arvore_otimizada(unsigned long freq[]) {
    NoLista* lista = NULL;
    NoHuffman* nos[TAM_ASCII];
    int count = 0;
    
    // Pré-alocação e contagem de símbolos usados
    for (int i = 0; i < TAM_ASCII; i++) {
        if (freq[i] > 0) {
            nos[count++] = criar_no(i, freq[i]);
        }
    }
    
    // Ordenação otimizada para pequenos conjuntos
    for (int i = 0; i < count-1; i++) {
        for (int j = i+1; j < count; j++) {
            if (nos[i]->frequencia > nos[j]->frequencia) {
                NoHuffman* temp = nos[i];
                nos[i] = nos[j];
                nos[j] = temp;
            }
        }
    }
    
    // Construção da árvore
    while (count > 1) {
        NoHuffman* esquerda = nos[0];
        NoHuffman* direita = nos[1];
        
        // Move os elementos restantes
        for (int i = 0; i < count-2; i++) {
            nos[i] = nos[i+2];
        }
        count -= 2;
        
        NoHuffman* pai = criar_no(0, esquerda->frequencia + direita->frequencia);
        pai->esquerda = esquerda;
        pai->direita = direita;
        
        // Insere ordenado
        int pos = count;
        while (pos > 0 && nos[pos-1]->frequencia > pai->frequencia) {
            nos[pos] = nos[pos-1];
            pos--;
        }
        nos[pos] = pai;
        count++;
    }
    
    return count > 0 ? nos[0] : NULL;
}

// Geração de códigos otimizada
void gerar_codigos_otimizado(NoHuffman* raiz, unsigned char codigo[], unsigned char tamanho) {
    if (!raiz->esquerda && !raiz->direita) {
        tabela_codigos[raiz->caractere] = malloc(tamanho);
        memcpy(tabela_codigos[raiz->caractere], codigo, tamanho);
        tamanho_codigos[raiz->caractere] = tamanho;
        return;
    }
    
    if (raiz->esquerda) {
        codigo[tamanho] = '0';
        gerar_codigos_otimizado(raiz->esquerda, codigo, tamanho+1);
    }
    
    if (raiz->direita) {
        codigo[tamanho] = '1';
        gerar_codigos_otimizado(raiz->direita, codigo, tamanho+1);
    }
}

// Serialização otimizada da árvore
void serializar_arvore_otimizada(NoHuffman* no, BitStream *bs, FILE *out, unsigned short *tamanho) {
    if (!no->esquerda && !no->direita) {
        write_bit(bs, 1, out); // Folha
        (*tamanho)++;
        for (int i = 7; i >= 0; i--) {
            write_bit(bs, (no->caractere >> i) & 1, out);
            (*tamanho)++;
        }
    } else {
        write_bit(bs, 0, out); // Nó interno
        (*tamanho)++;
        serializar_arvore_otimizada(no->esquerda, bs, out, tamanho);
        serializar_arvore_otimizada(no->direita, bs, out, tamanho);
    }
}

// ================= COMPACTAÇÃO PRINCIPAL =================

void compactar_arquivo(const char *input_filename, const char *output_filename) {
    FILE *in = fopen(input_filename, "rb");
    if (!in) {
        perror("Erro ao abrir arquivo de entrada");
        return;
    }

    // Análise do arquivo
    fseek(in, 0, SEEK_END);
    long original_size = ftell(in);
    rewind(in);

    if (original_size == 0) {
        fclose(in);
        printf("Erro: Arquivo vazio!\n");
        return;
    }

    // Cálculo de frequências com buffer grande
    unsigned long freq[TAM_ASCII] = {0};
    unsigned char buffer[BUFFER_SIZE];
    size_t bytes_lidos;
    
    while ((bytes_lidos = fread(buffer, 1, BUFFER_SIZE, in)) > 0) {
        for (size_t i = 0; i < bytes_lidos; i++) {
            freq[buffer[i]]++;
        }
    }
    rewind(in);

    // Construção da árvore otimizada
    NoHuffman* raiz = construir_arvore_otimizada(freq);
    if (!raiz) {
        fclose(in);
        printf("Erro ao construir árvore de Huffman\n");
        return;
    }

    // Geração dos códigos
    unsigned char codigo[256] = {0};
    gerar_codigos_otimizado(raiz, codigo, 0);

    // Arquivo de saída
    FILE *out = fopen(output_filename, "wb");
    if (!out) {
        fclose(in);
        liberar_arvore(raiz);
        perror("Erro ao abrir arquivo de saída");
        return;
    }

    // Header temporário
    HuffmanHeader header = {0};
    fwrite(&header, sizeof(header), 1, out);

    // Serialização da árvore
    BitStream bs;
    init_bit_stream(&bs);
    unsigned short tamanho_arvore = 0;
    serializar_arvore_otimizada(raiz, &bs, out, &tamanho_arvore);
    flush_bit_stream(&bs, out);

    // Atualiza header
    long pos_after_tree = ftell(out);
    header.tree_size = pos_after_tree - sizeof(header);

    // Compactação dos dados
    init_bit_stream(&bs);
    while ((bytes_lidos = fread(buffer, 1, BUFFER_SIZE, in)) > 0) {
        for (size_t i = 0; i < bytes_lidos; i++) {
            unsigned char c = buffer[i];
            for (unsigned char j = 0; j < tamanho_codigos[c]; j++) {
                write_bit(&bs, tabela_codigos[c][j] == '1' ? 1 : 0, out);
            }
        }
    }
    flush_bit_stream(&bs, out);

    // Calcula trash bits e atualiza header
    header.trash = (8 - bs.bits_preenchidos) % 8;
    fseek(out, 0, SEEK_SET);
    fwrite(&header, sizeof(header), 1, out);

    // Estatísticas
    fseek(out, 0, SEEK_END);
    long compressed_size = ftell(out);
    double taxa_compressao = (1.0 - (double)compressed_size/original_size) * 100;

    // Liberação de recursos
    fclose(in);
    fclose(out);
    liberar_arvore(raiz);
    for (int i = 0; i < TAM_ASCII; i++) {
        if (tabela_codigos[i]) free(tabela_codigos[i]);
    }

    printf("Compactação concluída:\n");
    printf("- Tamanho original: %ld bytes\n", original_size);
    printf("- Tamanho compactado: %ld bytes\n", compressed_size);
    printf("- Taxa de compressão: %.2f%%\n", taxa_compressao);
    printf("- Bits de lixo: %d\n", header.trash);
    printf("- Tamanho da árvore: %d bytes\n", header.tree_size);
}

// ================= DESCOMPACTAÇÃO =================

void descompactar_arquivo(const char *input_filename, const char *output_filename) {
    if (!input_filename || !output_filename) {
        printf("ERRO!\n");
        return;
    }
    
    FILE *in = fopen(input_filename, "rb");
    if (!in) {
        perror("Erro ao abrir arquivo compactado");
        return;
    }

    unsigned char header_bytes[2];
    if (fread(header_bytes, 1, 2, in) != 2) {
        fclose(in);
        printf("Erro ao ler cabeçalho do arquivo\n");
        return;
    }

    unsigned short header_value = (header_bytes[0] << 8) | header_bytes[1];
    int trash_bits = (header_value >> 13) & 0x7;
    unsigned short tree_size = header_value & 0x1FFF;

    NoHuffman* raiz = desserializar_arvore(in);
    if (!raiz) {
        fclose(in);
        printf("Erro ao reconstruir a árvore de Huffman\n");
        return;
    }

    long pos_after_tree = ftell(in);
    fseek(in, 0, SEEK_END);
    long file_size = ftell(in);
    fseek(in, pos_after_tree, SEEK_SET);

    FILE *out = fopen(output_filename, "wb");
    if (!out) {
        fclose(in);
        liberar_arvore(raiz);
        perror("Erro ao criar arquivo de saída");
        return;
    }

    NoHuffman* atual = raiz;
    unsigned char byte;
    long bytes_processados = 0;
    
    while (fread(&byte, 1, 1, in) == 1) {
        bytes_processados++;
        int is_last_byte = (bytes_processados == (file_size - pos_after_tree));
        int bits_to_process = is_last_byte ? (8 - trash_bits) : 8;

        for (int i = 7; i >= (8 - bits_to_process); i--) {
            int bit = (byte >> i) & 1;
            
            atual = bit ? atual->direita : atual->esquerda;
            
            if (!atual) {
                printf("Erro: Caminho inválido na árvore de Huffman\n");
                fclose(in);
                fclose(out);
                liberar_arvore(raiz);
                return;
            }

            if (!atual->esquerda && !atual->direita) {
                fputc(atual->caractere, out);
                atual = raiz;
            }
        }
    }

    fclose(in);
    fclose(out);
    liberar_arvore(raiz);
    printf("Arquivo descompactado com sucesso: %s\n", output_filename);
}

// ================= VERIFICAÇÃO DE INTEGRIDADE =================

void verificar_integridade(const char *arquivo_compactado, const char *arquivo_descompactado) {
    char nome_original[256];
    strncpy(nome_original, arquivo_compactado, strlen(arquivo_compactado) - 5);
    nome_original[strlen(arquivo_compactado) - 5] = '\0';
    
    FILE *original = fopen(nome_original, "rb");
    FILE *descompactado = fopen(arquivo_descompactado, "rb");
    
    if (!original || !descompactado) {
        if (!original) printf("Arquivo original %s nao encontrado\n", nome_original);
        if (!descompactado) printf("Arquivo descompactado %s nao encontrado\n", arquivo_descompactado);
        if (original) fclose(original);
        if (descompactado) fclose(descompactado);
        return;
    }

    int c1, c2;
    long pos = 0;
    int diferencas = 0;

    while ((c1 = fgetc(original)) != EOF && (c2 = fgetc(descompactado)) != EOF) {
        if (c1 != c2) {
            printf("Diferenca na posicao %ld: original=0x%02X, descompactado=0x%02X\n", 
                  pos, c1, c2);
            diferencas++;
        }
        pos++;
    }

    if (fgetc(original) != EOF || fgetc(descompactado) != EOF) {
        printf("AVISO: Os arquivos tem tamanhos diferentes!\n");
    }

    fclose(original);
    fclose(descompactado);

    if (diferencas == 0) {
        printf("Verificacao concluida: arquivos identicos\n");
    } else {
        printf("AVISO: Encontradas %d diferencas\n", diferencas);
    }
}

// ================= FUNÇÃO PRINCIPAL =================

int main() {
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
    if (scanf("%d", &opcao) != 1) {
        printf("Entrada inválida\n");
        return 1;
    }
    getchar();

    if (opcao == 1) {
        printf("Arquivo a compactar: ");
        if (!fgets(nome_arquivo, sizeof(nome_arquivo), stdin)) {
            printf("Erro ao ler entrada\n");
            return 1;
        }
        nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';

        strcpy(nome_original, nome_arquivo);
        strcat(nome_arquivo, ".huff");

        compactar_arquivo(nome_original, nome_arquivo);

    } else if (opcao == 2) {
        printf("Arquivo .huff a descompactar: ");
        if (!fgets(nome_arquivo, sizeof(nome_arquivo), stdin)) {
            printf("Erro ao ler entrada\n");
            return 1;
        }
        nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';

        if (!strstr(nome_arquivo, ".huff")) {
            printf("Deve ser um arquivo .huff\n");
            return 1;
        }

        strncpy(nome_saida, nome_arquivo, strlen(nome_arquivo) - 5);
        nome_saida[strlen(nome_arquivo) - 5] = '\0';
        strcat(nome_saida, ".dehuff");

        descompactar_arquivo(nome_arquivo, nome_saida);

        printf("\nDeseja verificar a integridade? (s/n): ");
        char resposta = getchar();
        if (resposta == 's' || resposta == 'S') {
            verificar_integridade(nome_arquivo, nome_saida);
        }
    } else if (opcao == 3) {
        printf("Arquivo .huff para verificar header: ");
        if (!fgets(nome_arquivo, sizeof(nome_arquivo), stdin)) {
            printf("Erro ao ler entrada\n");
            return 1;
        }
        nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';
        
        verificar_header(nome_arquivo);
    } else {
        printf("Opção inválida!\n");
        return 1;
    }

    return 0;
}
