#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct 
{
    int **clausulas;
    int num_clausulas;
    int num_literais;
} Formula;

typedef struct BinaryTree
{
    int valor;
    int variavel;
    struct BinaryTree *esquerda;
    struct BinaryTree *direita;
} BinaryTree;

// Função para liberar a árvore binária recursivamente
void liberar_arvore(BinaryTree *no) 
{
    if (no == NULL) return;
    liberar_arvore(no->esquerda);
    liberar_arvore(no->direita);
    free(no);
}

// Função para ler a fórmula 
Formula* ler_formula(const char *file) 
{
    FILE *arquivo = fopen(file, "r");
    if (!arquivo) 
    {
        printf("Erro ao abrir o arquivo");
        return NULL;
    }

    Formula *f = malloc(sizeof(Formula));
    f->clausulas = NULL;
    f->num_clausulas = 0;
    f->num_literais = 0;

    char linha[100];
    int clausula_atual = 0;

    while (fgets(linha, sizeof(linha), arquivo)) 
    {
        if (linha[0] == 'c') continue; // Comentário
        if (linha[0] == 'p') 
        {
            sscanf(linha, "p cnf %d %d", &f->num_literais, &f->num_clausulas); //retorna o numero de itens de entrada, ou seja, "%d" vai ler um inteiro e retorna-lo
            f->clausulas = malloc(f->num_clausulas * sizeof(int*)); //alocando espaço para o vetor de ponteiros, isto é, para as linhas da matriz
            continue;
        }

        // Lê uma cláusula
        int *clausula = malloc(51 * sizeof(int)); // alocando memoria pra 31 literais + 0
        int literal, tamanho = 0;
        char *token = strtok(linha, " \t\n"); // strtok é uma funçao q separa a string (linha) com base nos caracteres definidos como delimitadores,nesse caso, espaço(' '), tab(\t) e quebra de linha (\n)
        // ex: "1 -2 3 0\n" 
        // strtok faz isso: token = "1"; token = "-2"; token = "3"; token = "0"; token = NULL;

 
        while (token != NULL) 
        {
            literal = atoi(token); //transforma a string(token) em um inteiro, ou seja, exemplo: "1" -> 1
            if (literal == 0) break; // se o literal for 0, é o fim da clausula
            clausula[tamanho++] = literal; //adiciona o literal no vetor clausula
            token = strtok(NULL, " \t\n"); // pega o próximo token da linha (literal seguinte)
        }
        // Garanta que só há um zero:
        clausula[tamanho] = 0;  // Único terminador

        f->clausulas[clausula_atual++] = clausula; // o que essa linha esta dizendo: "A clausula_atual-ésima posição do vetor de cláusulas (int **) vai apontar para o mesmo lugar que o ponteiro clausula aponta."
    }

    fclose(arquivo);
    return f;
}

void liberar_formula(Formula *f) 
{
    for (int i = 0; i < f->num_clausulas; i++) 
    {
        free(f->clausulas[i]);
    }
    free(f->clausulas);
    free(f);
}

bool clausula_satisfeita(int *clausula, int *interpretacao) 
{
    for (int i = 0; clausula[i] != 0; i++) 
    {
        int var = abs(clausula[i]);
        int valor = clausula[i] > 0 ? 1 : -1;
        if (interpretacao[var] == valor) 
        {
            return true;  // Se a cláusula for satisfeita por um literal, retorna true
        }
    }
    return false;  // Se nenhuma condição for satisfeita, retorna false
}


bool formula_satisfativel(Formula *f, int *interpretacao) 
{
    for (int i = 0; i < f->num_clausulas; i++) 
    {
        if (!clausula_satisfeita(f->clausulas[i], interpretacao)) 
        {
            return false;
        }
    }
    return true;
}

bool formula_insatisfativel(Formula *f, int *interpretacao) 
{
    for (int i = 0; i < f->num_clausulas; i++) 
    {
        bool todos_falsos = true;
        for (int j = 0; f->clausulas[i][j] != 0; j++) 
        {
            int var = abs(f->clausulas[i][j]);
            int valor = f->clausulas[i][j] > 0 ? 1 : -1;
            if (interpretacao[var] == 0 || interpretacao[var] == valor) 
            {
                todos_falsos = false;
                break;
            }
        }
        if (todos_falsos) return true;
    }
    return false;
}

int proxima_variavel_nao_atribuida(Formula *f, int *interpretacao) 
{
    for (int i = 1; i <= f->num_literais; i++) 
    {
        if (interpretacao[i] == 0) 
        {
            return i;
        }
    }
    return 0;
}

bool SAT(Formula *f, int *interpretacao, BinaryTree *no)
{
    if (formula_satisfativel(f, interpretacao)) return true;
    if (formula_insatisfativel(f, interpretacao)) return false;

    int var = proxima_variavel_nao_atribuida(f, interpretacao);
    if (var == 0) return false;

    // Primeiro tenta verdadeiro (esquerda)
    interpretacao[var] = 1;
    no->esquerda = malloc(sizeof(BinaryTree));
    no->esquerda->variavel = var;
    no->esquerda->valor = 1;
    no->esquerda->esquerda = no->esquerda->direita = NULL;

    if (SAT(f, interpretacao, no->esquerda)) return true;

    // Depois tenta falso (direita)
    interpretacao[var] = -1;
    no->direita = malloc(sizeof(BinaryTree));
    no->direita->variavel = var;
    no->direita->valor = -1;
    no->direita->esquerda = no->direita->direita = NULL;

    if (SAT(f, interpretacao, no->direita)) return true;

    // Backtrack
    interpretacao[var] = 0;
    return false;
}


int main() 
{
    const char *arquivo = "SAT.cnf";
    Formula *f = ler_formula(arquivo); //chamando funcao pra ler o arquivo SAT
    if (!f) 
    {
        printf("Erro ao carregar o arquivo ou arquivo vazio!\n");
        return 1;
    }

    int *interpretacao = calloc(f->num_literais + 1, sizeof(int)); // calloc aloca memória e inicializa todos os bytes com 0
    //f->num_literais + 1 : define o tamanho do vetor
    if (!interpretacao) 
    {
        printf("Falha ao alocar interpretacao!\n");
        return 1;
    }

    BinaryTree *raiz = malloc(sizeof(BinaryTree));
    if (!raiz) 
    {
        printf("Falha ao alocar raiz!\n");
        free(interpretacao);
        return 1;
    }
    raiz->variavel = 0;
    raiz->valor = 0;
    raiz->esquerda = NULL;
    raiz->direita = NULL;

    if (SAT(f, interpretacao, raiz)) 
    {
        printf("SAT\nInterpretacao:");
        for (int i = 1; i <= f->num_literais; i++) 
        {
            printf(" x%d=%d", i, interpretacao[i] > 0 ? 1 : 0);
        }
        printf("\n");
    } 
    else 
    {
        printf("UNSAT\n");
    }

    liberar_formula(f);
    free(interpretacao);
    liberar_arvore(raiz);
    return 0;
}