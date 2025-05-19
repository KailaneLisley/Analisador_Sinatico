#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TAMANHO_TABELA 100      // Tamanho máximo da tabela de hash
#define MAX_TAM_TOKEN 50        // Tamanho máximo para o nome do token
#define MAX_TAM_LEXEMA 100      // Tamanho máximo para o lexema
#define MAX_TAM_CATEGORIA 50    // Tamanho máximo para a categoria

// Estrutura para representar um token
typedef struct Token {
    char nome[MAX_TAM_TOKEN];
    char lexema[MAX_TAM_LEXEMA];
    char categoria[MAX_TAM_CATEGORIA];
    struct Token* prox;  // Para tratamento de colisões
} Token;

// Estrutura da Tabela hash
typedef struct {
    Token* tabela[TAMANHO_TABELA];
} TabelaHash;

unsigned int funcaoHash(const char* nome_token);
void inicializarTabelaH(TabelaHash* TH);
void inserirToken(TabelaHash* TH, const char* nome, const char* lexema, const char* categoria);
Token* buscarToken(TabelaHash* ht, const char* nome);
int lerTokensArquivo(TabelaHash* ht, const char* filename);
void liberarTabela(TabelaHash* TH);
void printTokens(TabelaHash* TH);

int main() {
    TabelaHash TabelaToken;
    inicializarTabelaH(&TabelaToken);
    
    // Carregar tokens do arquivo
    int tokenAtual = lerTokensArquivo(&TabelaToken, "tokens.txt");
    printf("Carregados %d tokens do arquivo.\n\n", tokenAtual);
    
    // Buscar alguns tokens para teste
    const char* encontrarToken[] = {"INT", "IF", "COMP", "ID", "NUM_INT"};
    int numTokensToFind = sizeof(encontrarToken) / sizeof(encontrarToken[0]);
    
    for (int i = 0; i < numTokensToFind; i++) {
        Token* token = buscarToken(&TabelaToken, encontrarToken[i]);
        if (token != NULL) {
            printf("Token encontrado: %s, Lexema: %s, Categoria: %s\n", 
                   token->nome, token->lexema, token->categoria);
        } else {
            printf("Token não encontrado: %s\n", encontrarToken[i]);
        }
    }
    
    // Opcional: imprimir todos os tokens
    // printAllTokens(&tokenTable);
    printTokens(&TabelaToken);
    // Liberar memória
    liberarTabela(&TabelaToken);
    
    return 0;
}

// Função de hash
unsigned int funcaoHash(const char* nome_token){
    unsigned int hash = 0;
    for (int i = 0; nome_token[i] != '\0'; i++) {
        hash = hash * 31 + nome_token[i];
    }
    return hash % TAMANHO_TABELA;
}

// Inicializar tabela hash
void inicializarTabelaH(TabelaHash* TH){
    for (int i = 0; i < TAMANHO_TABELA; i++) {
        TH->tabela[i] = NULL;
    }
}

// Inserir token na tabela hash
void inserirToken(TabelaHash* TH, const char* nome, const char* lexema, const char* categoria){
    unsigned int indice = funcaoHash(nome);
    // Criar novo token
    Token* novoToken = (Token*)malloc(sizeof(Token));
    if (novoToken == NULL) {
        fprintf(stderr, "Erro: Falha na alocação de memória\n");
        return;
    }
    strcpy(novoToken->nome, nome);
    strcpy(novoToken->lexema, lexema);
    strcpy(novoToken->categoria, categoria);
    novoToken->prox = NULL;

    // Inserir na tabela (com tratamento de colisão por encadeamento)
    if (TH->tabela[indice] == NULL) {
        TH->tabela[indice] = novoToken;
    } else {
        // Adicionar no início da lista encadeada
        novoToken->prox = TH->tabela[indice];
        TH->tabela[indice] = novoToken;
    }
}

// Buscar token na tabela hash
Token* buscarToken(TabelaHash* TH, const char* nome){
    unsigned int indice = funcaoHash(nome);
    Token* atual= TH->tabela[indice];
    
    while (atual != NULL) {
        if (strcmp(atual->nome, nome) == 0) {
            return atual;
        }
        atual = atual->prox;
    }
    return NULL;  // Token não encontrado
}

// Carregar tokens de um arquivo para a tabela hash
int lerTokensArquivo(TabelaHash* TH, const char* arquivo){
    FILE* arq = fopen(arquivo, "r");
    if (arq == NULL) {
        fprintf(stderr, "Erro: Não foi possível abrir o arquivo %s\n", arquivo);
        return 0;
    }
    char nome_token[MAX_TAM_TOKEN]; 
    char lexema[MAX_TAM_LEXEMA];
    char categoria[MAX_TAM_CATEGORIA];
    int count = 0;
    
    // Ler cada linha do arquivo
    while (fscanf(arq, "%s %s %s", nome_token, lexema, categoria) == 3) {
        inserirToken(TH, nome_token, lexema, categoria);
        count++;
    }
    
    fclose(arq);
    return count;
}

// Liberar memória da tabela hash
void liberarTabela(TabelaHash* TH){
    for (int i = 0; i < TAMANHO_TABELA; i++) {
        Token* atual = TH->tabela[i];
        while (atual != NULL) {
            Token* aux = atual;
            atual = atual->prox;
            free(aux);
        }
        TH->tabela[i] = NULL;
    }
}

// Imprimir todos os tokens da tabela hash (para depuração)
void printTokens(TabelaHash* TH){
    printf("Conteúdo da Tabela Hash:\n");
    printf("------------------------\n");
    
    for (int i = 0; i < TAMANHO_TABELA; i++) {
        Token* atual = TH->tabela[i];
        if (atual != NULL) {
            printf("Índice %d:\n", i);
            while (atual != NULL) {
                printf("  Token: %s, Lexema: %s, Categoria: %s\n", 
                       atual->nome, atual->lexema, atual->categoria);
                atual = atual->prox;
            }
        }
    }
}