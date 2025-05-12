#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
 

// Definindo a estrutura da fila
struct spool
{
    char arquivo[50];  // Armazenando o nome do arquivo como string
    struct spool *next;
} *head = NULL; // A fila de impressão começa vazia


// Verifica se a fila está vazia
int fila_vazia()
{
    return head == NULL;
}

// Retira o arquivo da fila
void retirar_arquivo()
{
    if(fila_vazia())
    {
        printf("Fila vazia\n");
        return;
    }

    struct spool *temp = head;
    head = head->next;  // Remove o primeiro item da fila
    free(temp);         // Libera a memória do arquivo removido
}

// Imprime o arquivo
void imprimir_arquivo(const char *arq)
{
    if(fila_vazia())
    {
        printf("Nenhum arquivo na fila de impressao\n");
        return;
    }

    printf("----- Inicio da impressao -----\n\n");
    FILE *f = fopen(arq, "r");

    if (f == NULL) {
        printf("Erro ao abrir o arquivo: %s\n", arq);
        return;
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), f)) 
    {
        printf("%s", linha);  // Imprime linha por linha
    }

    fclose(f);  // Fecha o arquivo após a impressão

    printf("\n----- Fim da impressao -----\n\n");
}

// Adiciona um novo arquivo à fila
void adicionar_arquivo(const char *arq)
{
    FILE *f = fopen(arq, "r"); //abre o arquivo pra leitura
    if (f == NULL) 
    {
        printf("Erro: Arquivo não encontrado ou sem permissão.\n");
        return;
    }
    fclose(f);

    struct spool *new_node = malloc(sizeof(struct spool));
    if (new_node == NULL) 
    {
        printf("Erro ao alocar memória para o arquivo.\n");
        return;
    }

    strcpy(new_node->arquivo, arq);  // Copia o nome do arquivo para o nó
    new_node->next = NULL;

    if (head == NULL) 
    {
        head = new_node;  // Se a fila está vazia, o novo arquivo é o primeiro
    } 
    else 
    {
        struct spool *temp = head;
        while (temp->next != NULL) 
        {  // Vai até o final da fila
            temp = temp->next;
        }
        temp->next = new_node;  // Adiciona o novo arquivo no final da fila
    }
}

// Lista todos os arquivos na fila
void listar_fila() 
{
    if(fila_vazia()) 
    {
        printf("Fila de impressao vazia.\n");
        return;
    }

    printf("\n--- Fila de Impressao ---\n\n");
    struct spool *temp = head;
    int posicao = 1;
    while(temp != NULL) 
    {
        printf("%d. %s\n", posicao++, temp->arquivo);
        temp = temp->next;
    }
    printf("\n-------------------------\n\n");
}


// Função para limpar a fila completamente
void limpar_fila() 
{
    while(!fila_vazia()) 
    {
        retirar_arquivo();
    }
    printf("Fila de impressao limpa.\n");
}

// Exibe o menu de opções
void exibir_menu() 
{
    printf("\n===== Sistema de Impressao =====\n");
    printf("1. Adicionar arquivo na fila de impressao\n");
    printf("2. Imprimir proximo arquivo\n");
    printf("3. Listar arquivos na fila\n");
    printf("4. Limpar fila de impressao\n");
    printf("0. Sair\n");
    printf("===============================\n");
    printf("Opcao: ");
}

// Função principal
int main() 
{
    int escolha;
    char arquivo[50];

    do 
    {
        exibir_menu();
        scanf("%d", &escolha);

        switch (escolha) 
        {
            case 1:

                printf("Digite o nome do arquivo: ");
                scanf("%s", arquivo);
                adicionar_arquivo(arquivo);
                break;

            case 2:

                if (!fila_vazia()) //se a fila nao estiver vazia o if é verdadeiro
                {
                    imprimir_arquivo(head->arquivo);
                    retirar_arquivo();
                } 
                else 
                {
                    printf("Nenhum arquivo na fila de impressao.\n");
                }
                break;

            case 3:

                listar_fila();
                break;

            case 4:

                limpar_fila();
                break;

            case 0:

                printf("Saindo do sistema...\n");
                limpar_fila(); // Limpa a fila antes de sair
                break;

            default:

                printf("Opcao invalida! Tente novamente.\n");
                // Limpa o buffer de entrada para evitar loops infinitos
                while(getchar() != '\n');
                break;
        }

    } while (escolha != 0);

    return 0;
}

// o que é spool de impressão? 
// sistema que gerencia trabalhos de impressao em um buffer (Memória ou arquivo temporário que armazena os dados antes da impressão)
