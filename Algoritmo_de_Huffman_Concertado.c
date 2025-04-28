#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <locale.h>

#define TAM_ASCII 256

typedef struct {
    unsigned char trash : 3;
    unsigned short tree_size : 13;
} HuffmanHeader;

typedef struct NoHuffman {
    unsigned char caractere;
    unsigned frequencia;
    struct NoHuffman *esquerda, *direita;
} NoHuffman;

typedef struct NoLista {
    void *dados;
    struct NoLista *prox;
} NoLista;

char* tabela_codigos[TAM_ASCII];

// ================= FUNÇÕES AUXILIARES =================

void inicializar_tabela_com_zeros(unsigned int tabela[]) {
    memset(tabela, 0, TAM_ASCII * sizeof(unsigned int));
}

void preencher_tabela_de_frequencia_arquivo(FILE* arquivo, unsigned int tabela[]) {
    unsigned char byte;
    while (fread(&byte, 1, 1, arquivo) == 1) {
        tabela[byte]++;
    }
    rewind(arquivo);
}

NoHuffman* criar_no(unsigned char caractere, unsigned int frequencia) {
    NoHuffman* novo = (NoHuffman*)malloc(sizeof(NoHuffman));
    if (!novo) return NULL;
    
    novo->caractere = caractere;
    novo->frequencia = frequencia;
    novo->esquerda = novo->direita = NULL;
    return novo;
}

void liberar_arvore(NoHuffman* raiz) {
    if (!raiz) return;
    liberar_arvore(raiz->esquerda);
    liberar_arvore(raiz->direita);
    free(raiz);
}

int comparar_frequencia(void* a, void* b) {
    NoHuffman* n1 = (NoHuffman*)a;
    NoHuffman* n2 = (NoHuffman*)b;
    return n1->frequencia - n2->frequencia;
}

NoLista* criar_lista(unsigned int freq[]) {
    NoLista* lista = NULL;
    for (int i = 0; i < TAM_ASCII; i++) {
        if (freq[i] > 0) {
            NoHuffman* novo = criar_no(i, freq[i]);
            if (!novo) continue;
            
            NoLista* novo_no = (NoLista*)malloc(sizeof(NoLista));
            if (!novo_no) {
                free(novo);
                continue;
            }
            
            novo_no->dados = novo;
            novo_no->prox = lista;
            lista = novo_no;
        }
    }
    return lista;
}

NoLista* inserir_ordenado(NoLista* lista, void* dados, int (*comparar)(void*, void*)) {
    if (!dados) return lista;
    
    NoLista* novo = (NoLista*)malloc(sizeof(NoLista));
    if (!novo) return lista;
    
    novo->dados = dados;
    novo->prox = NULL;

    if (!lista || comparar(dados, lista->dados) < 0) {
        novo->prox = lista;
        return novo;
    }

    NoLista* atual = lista;
    while (atual->prox && comparar(dados, atual->prox->dados) >= 0) {
        atual = atual->prox;
    }

    novo->prox = atual->prox;
    atual->prox = novo;
    return lista;
}

NoLista* remover_inicio(NoLista* lista) {
    if (!lista) return NULL;
    NoLista* temp = lista;
    lista = lista->prox;
    free(temp);
    return lista;
}

NoHuffman* construir_arvore(NoLista* lista) {
    while (lista && lista->prox) {
        NoHuffman* esquerda = (NoHuffman*)lista->dados;
        lista = remover_inicio(lista);
        
        NoHuffman* direita = (NoHuffman*)lista->dados;
        lista = remover_inicio(lista);
        
        NoHuffman* pai = criar_no(0, esquerda->frequencia + direita->frequencia);
        if (!pai) {
            liberar_arvore(esquerda);
            liberar_arvore(direita);
            continue;
        }
        
        pai->esquerda = esquerda;
        pai->direita = direita;
        
        lista = inserir_ordenado(lista, pai, comparar_frequencia);
    }
    return lista ? (NoHuffman*)lista->dados : NULL;
}

// ================= SERIALIZAÇÃO DA ÁRVORE =================

void serializar_arvore(NoHuffman* no, FILE* saida, unsigned short *tamanho) {
    if (!no || !saida || !tamanho) return;

    if (!no->esquerda && !no->direita) {
        fputc('1', saida);
        (*tamanho)++;
        fputc(no->caractere, saida);
        (*tamanho)++;
    } else {
        fputc('0', saida);
        (*tamanho)++;
        serializar_arvore(no->esquerda, saida, tamanho);
        serializar_arvore(no->direita, saida, tamanho);
    }
}

NoHuffman* desserializar_arvore(FILE* entrada) {
    if (!entrada) return NULL;
    
    int c = fgetc(entrada);
    if (c == EOF) return NULL;

    if (c == '1') {
        c = fgetc(entrada);
        if (c == EOF) return NULL;
        return criar_no((unsigned char)c, 0);
    } else if (c == '0') {
        NoHuffman* no = criar_no(0, 0);
        if (!no) return NULL;
        
        no->esquerda = desserializar_arvore(entrada);
        no->direita = desserializar_arvore(entrada);
        return no;
    }

    return NULL;
}

// ================= CODIFICAÇÃO =================

void gerar_tabela_codigos(NoHuffman* raiz, char* caminho, char* tabela[]) {
    if (!raiz || !tabela) return;
    
    if (!raiz->esquerda && !raiz->direita) {
        tabela[raiz->caractere] = strdup(caminho ? caminho : "");
        return;
    }
    
    char caminho_esq[256] = {0};
    char caminho_dir[256] = {0};
    
    if (caminho) {
        strcpy(caminho_esq, caminho);
        strcpy(caminho_dir, caminho);
    }
    
    strcat(caminho_esq, "0");
    strcat(caminho_dir, "1");
    
    gerar_tabela_codigos(raiz->esquerda, caminho_esq, tabela);
    gerar_tabela_codigos(raiz->direita, caminho_dir, tabela);
}

// ================= COMPACTAÇÃO =================

void compactar_arquivo(const char *input_filename, const char *output_filename) {
    if (!input_filename || !output_filename) return;
    
    FILE *in = fopen(input_filename, "rb");
    if (!in) {
        perror("Erro ao abrir arquivo de entrada");
        return;
    }

    // Calcula frequências
    unsigned int freq[TAM_ASCII] = {0};
    preencher_tabela_de_frequencia_arquivo(in, freq);

    // Constrói árvore de Huffman
    NoLista* lista = criar_lista(freq);
    NoHuffman* raiz = construir_arvore(lista);
    if (!raiz) {
        fclose(in);
        printf("Erro ao construir árvore de Huffman\n");
        return;
    }

    // Gera tabela de códigos
    memset(tabela_codigos, 0, sizeof(tabela_codigos));
    gerar_tabela_codigos(raiz, "", tabela_codigos);

    FILE *out = fopen(output_filename, "wb");
    if (!out) {
        fclose(in);
        liberar_arvore(raiz);
        perror("Erro ao abrir arquivo de saída");
        return;
    }

    // Escreve espaço para o header
    fputc(0, out);
    fputc(0, out);

    // Serializa a árvore
    unsigned short tamanho_arvore = 0;
    serializar_arvore(raiz, out, &tamanho_arvore);

    // Codifica o conteúdo
    unsigned char buffer = 0;
    int bits_preenchidos = 0;
    unsigned char byte;
    
    rewind(in);
    while (fread(&byte, 1, 1, in) == 1) {
        char *codigo = tabela_codigos[byte];
        if (!codigo) continue;
        
        for (size_t i = 0; codigo[i] != '\0'; i++) {
            buffer <<= 1;
            if (codigo[i] == '1') {
                buffer |= 1;
            }
            bits_preenchidos++;
            
            if (bits_preenchidos == 8) {
                fwrite(&buffer, 1, 1, out);
                buffer = 0;
                bits_preenchidos = 0;
            }
        }
    }

    // Trata bits restantes e atualiza header
    int trash_bits = 0;
    if (bits_preenchidos > 0) {
        trash_bits = 8 - bits_preenchidos;
        buffer <<= trash_bits;
        fwrite(&buffer, 1, 1, out);
    }

    // Atualiza o header
    fseek(out, 0, SEEK_SET);
    unsigned short header_value = (trash_bits << 13) | (tamanho_arvore & 0x1FFF);
    fputc((header_value >> 8) & 0xFF, out);
    fputc(header_value & 0xFF, out);

    fclose(in);
    fclose(out);
    liberar_arvore(raiz);
}

// ================= DESCOMPACTAÇÃO =================

void descompactar_arquivo(const char *input_filename, const char *output_filename) {
    if (!input_filename || !output_filename) return;
    
    FILE *in = fopen(input_filename, "rb");
    if (!in) {
        perror("Erro ao abrir arquivo compactado");
        return;
    }

    // Lê o header
    unsigned char header_bytes[2];
    if (fread(header_bytes, 1, 2, in) != 2) {
        fclose(in);
        printf("Erro ao ler cabeçalho do arquivo\n");
        return;
    }

    unsigned short header_value = (header_bytes[0] << 8) | header_bytes[1];
    int trash_bits = (header_value >> 13) & 0x7;
    int tree_size = header_value & 0x1FFF;

    // Desserializa a árvore
    NoHuffman* raiz = desserializar_arvore(in);
    if (!raiz) {
        fclose(in);
        printf("Erro ao reconstruir a árvore de Huffman\n");
        return;
    }

    // Verifica tamanho do arquivo
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
    
    while (fread(&byte, 1, 1, in) == 1) {
        int is_last_byte = (ftell(in) == file_size);
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

// ================= FUNÇÃO PRINCIPAL =================

int main() {
    setlocale(LC_ALL, "Portuguese");

    int opcao;
    char nome_arquivo[256] = {0};
    char nome_saida[256] = {0};

    printf("Huffman File Compressor\n");
    printf("1. Compactar arquivo\n");
    printf("2. Descompactar arquivo\n");
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

        strncpy(nome_saida, nome_arquivo, sizeof(nome_saida) - 5);
        strcat(nome_saida, ".huff");

        compactar_arquivo(nome_arquivo, nome_saida);
        printf("Arquivo compactado: %s\n", nome_saida);

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

        strncpy(nome_saida, nome_arquivo, sizeof(nome_saida) - 5);
        char* ext = strstr(nome_saida, ".huff");
        if (ext) *ext = '\0';
        
        // Mantém a extensão original se existia antes do .huff
        char* original_ext = strrchr(nome_arquivo, '.');
        if (original_ext && strcmp(original_ext, ".huff") != 0) {
            strcat(nome_saida, original_ext);
        }

        descompactar_arquivo(nome_arquivo, nome_saida);
    } else {
        printf("Opção inválida!\n");
        return 1;
    }

    // Libera a tabela de códigos
    for (int i = 0; i < TAM_ASCII; i++) {
        if (tabela_codigos[i]) {
            free(tabela_codigos[i]);
            tabela_codigos[i] = NULL;
        }
    }

    return 0;
}
