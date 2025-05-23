
#include <stdio.h>
#include <string.h>

#define NUM_NAO_TERMINAIS 17
#define NUM_TERMINAIS 42
#define MAX_PILHA 100
#define MAX_TOKENS 100

// Enum para não-terminais
enum {
    PROGRAMA, LISTA_DECL, DECL, DECL_VAR, DECL_FUN,
    TIPO, PARAMS, PARAM, COMP_DECL, LISTA_COMANDOS,
    COMANDO, EXP, EXPR_SIMPLES, SOMA, TERMO, FATOR,
    PROGRAMA_FIM
};

// Enum para terminais
enum {
    INT, FLOAT, DOUBLE, CHAR, BOOLEAN, VOID, ID, ABRE_PAREN, FECHA_PAREN,
    ABRE_CHAVE, FECHA_CHAVE, PONTO_VIRGULA, ATRIBUICAO, IGUAL, DIFERENTE,
    MAIOR, MAIOR_IGUAL, MENOR, MENOR_IGUAL, MAIS, MENOS, MULT, DIV, MOD,
    AND, OR, IF, ELSE, WHILE, FOR, SWITCH, BREAK, CONTINUE, RETURN,
    CASE, DEFAULT, ABRE_COLCHETE, FECHA_COLCHETE, TEXTO, COMENTARIO, NUM, EPSILON
};

// Pilha para símbolos
int pilha[MAX_PILHA];
int topo = -1;

// Tabela LL(1)
int tabelaM[NUM_NAO_TERMINAIS][NUM_TERMINAIS];

// Funções de manipulação de pilha
void empilhar(int simbolo) {
    if (topo < MAX_PILHA - 1)
        pilha[++topo] = simbolo;
    else
        printf("Erro: pilha cheia\n");
}

int desempilhar() {
    if (topo >= 0)
        return pilha[topo--];
    else {
        printf("Erro: pilha vazia\n");
        return -1;
    }
}

// Inicialização da tabela M 
void inicializarTabelaM() {
    // Inicializa toda a tabela com -1 (sem produção)
    for (int i = 0; i < NUM_NAO_TERMINAIS; i++)
        for (int j = 0; j < NUM_TERMINAIS; j++)
            tabelaM[i][j] = -1;

    tabelaM[PROGRAMA][INT] = 0;
    tabelaM[PROGRAMA][FLOAT] = 0;
    tabelaM[PROGRAMA][DOUBLE] = 0;
    tabelaM[PROGRAMA][CHAR] = 0;
    tabelaM[PROGRAMA][BOOLEAN] = 0;
    tabelaM[PROGRAMA][VOID] = 0;

    tabelaM[LISTA_DECL][INT] = 1;
    tabelaM[LISTA_DECL][FLOAT] = 1;
    tabelaM[LISTA_DECL][DOUBLE] = 1;
    tabelaM[LISTA_DECL][CHAR] = 1;
    tabelaM[LISTA_DECL][BOOLEAN] = 1;
    tabelaM[LISTA_DECL][VOID] = 1;

    tabelaM[DECL][INT] = 2;
    tabelaM[DECL][FLOAT] = 2;
    tabelaM[DECL][DOUBLE] = 2;
    tabelaM[DECL][CHAR] = 2;
    tabelaM[DECL][BOOLEAN] = 2;
    tabelaM[DECL][VOID] = 3;

    tabelaM[DECL_VAR][INT] = 4;
    tabelaM[DECL_VAR][FLOAT] = 4;
    tabelaM[DECL_VAR][DOUBLE] = 4;
    tabelaM[DECL_VAR][CHAR] = 4;
    tabelaM[DECL_VAR][BOOLEAN] = 4;

    tabelaM[DECL_FUN][VOID] = 5;

    tabelaM[TIPO][INT] = 6;
    tabelaM[TIPO][FLOAT] = 7;
    tabelaM[TIPO][DOUBLE] = 8;
    tabelaM[TIPO][CHAR] = 9;
    tabelaM[TIPO][BOOLEAN] = 10;

    tabelaM[PARAMS][INT] = 11;
    tabelaM[PARAMS][FLOAT] = 11;
    tabelaM[PARAMS][DOUBLE] = 11;
    tabelaM[PARAMS][CHAR] = 11;
    tabelaM[PARAMS][BOOLEAN] = 11;
    tabelaM[PARAMS][FECHA_PAREN] = 12;

    tabelaM[PARAM][INT] = 13;
    tabelaM[PARAM][FLOAT] = 13;
    tabelaM[PARAM][DOUBLE] = 13;
    tabelaM[PARAM][CHAR] = 13;
    tabelaM[PARAM][BOOLEAN] = 13;

    tabelaM[COMP_DECL][ABRE_CHAVE] = 14;

    tabelaM[LISTA_COMANDOS][ID] = 15;
    tabelaM[LISTA_COMANDOS][IF] = 15;
    tabelaM[LISTA_COMANDOS][WHILE] = 15;
    tabelaM[LISTA_COMANDOS][RETURN] = 15;
    tabelaM[LISTA_COMANDOS][ABRE_CHAVE] = 15;
    tabelaM[LISTA_COMANDOS][FECHA_CHAVE] = 16;

    tabelaM[COMANDO][ID] = 17;
    tabelaM[COMANDO][IF] = 18;
    tabelaM[COMANDO][WHILE] = 19;
    tabelaM[COMANDO][RETURN] = 20;
    tabelaM[COMANDO][ABRE_CHAVE] = 21;

    tabelaM[EXP][ID] = 22;
    tabelaM[EXP][NUM] = 22;
    tabelaM[EXP][ABRE_PAREN] = 22;

    tabelaM[EXPR_SIMPLES][ID] = 23;
    tabelaM[EXPR_SIMPLES][NUM] = 23;
    tabelaM[EXPR_SIMPLES][ABRE_PAREN] = 23;

    tabelaM[SOMA][MAIS] = 24;
    tabelaM[SOMA][MENOS] = 25;
    tabelaM[SOMA][EPSILON] = 26;  // Exemplo para epsilon

    tabelaM[TERMO][ID] = 27;
    tabelaM[TERMO][NUM] = 27;
    tabelaM[TERMO][ABRE_PAREN] = 27;

    tabelaM[FATOR][ID] = 28;
    tabelaM[FATOR][NUM] = 29;
    tabelaM[FATOR][ABRE_PAREN] = 30;
}

// Produções da gramática 
void aplicarProducao(int producao) {
    switch (producao) {
        case 0: 
            empilhar(PROGRAMA_FIM); 
            empilhar(LISTA_DECL); 
            break;
        case 1: 
            empilhar(LISTA_DECL); 
            empilhar(DECL); 
            break;
        case 2: 
            empilhar(DECL_VAR); 
            break;
        case 3: 
            empilhar(DECL_FUN); 
            break;
        case 4: 
            empilhar(PONTO_VIRGULA); 
            empilhar(ID); 
            empilhar(TIPO); 
            break;
        case 5: 
            empilhar(COMP_DECL); 
            empilhar(FECHA_PAREN); 
            empilhar(PARAMS); 
            empilhar(ABRE_PAREN); 
            empilhar(ID); 
            empilhar(VOID); 
            break;
        case 6: 
            empilhar(INT); 
            break;
        case 7: 
            empilhar(FLOAT); 
            break;
        case 8: 
            empilhar(DOUBLE); 
            break;
        case 9: 
            empilhar(CHAR); 
            break;
        case 10: 
            empilhar(BOOLEAN); 
            break;
        case 11: 
            empilhar(PARAM); 
            break;
        case 12: 
            /* produção vazia */ 
            break;
        case 13: 
            empilhar(ID); 
            empilhar(TIPO); 
            break;
        case 14: 
            empilhar(FECHA_CHAVE); 
            empilhar(LISTA_COMANDOS); 
            empilhar(ABRE_CHAVE); 
            break;
        case 15: 
            empilhar(LISTA_COMANDOS); 
            empilhar(COMANDO); 
            break;
        case 16: 
            /* produção vazia */ 
            break;
        case 17: 
            empilhar(PONTO_VIRGULA); 
            empilhar(EXP); 
            empilhar(ATRIBUICAO); 
            empilhar(ID); 
            break;
        case 18: 
            empilhar(COMANDO); 
            empilhar(FECHA_PAREN); 
            empilhar(EXP); 
            empilhar(ABRE_PAREN); 
            empilhar(IF); 
            break;
        case 19: 
            empilhar(COMANDO); 
            empilhar(FECHA_PAREN); 
            empilhar(EXP); 
            empilhar(ABRE_PAREN);
			empilhar(WHILE);
			break;
		case 20:
			empilhar(PONTO_VIRGULA);
			empilhar(EXP);
			empilhar(RETURN);
			break;
		case 21:
			empilhar(COMP_DECL);
			break;
		case 22:
			empilhar(EXPR_SIMPLES);
			break;
		case 23:
			empilhar(TERMO);
			empilhar(SOMA);
			break;
		case 24:
			empilhar(SOMA);
			empilhar(TERMO);
			empilhar(MAIS);
			break;
		case 25:
			empilhar(SOMA);
			empilhar(TERMO);
			empilhar(MENOS);
			break;
			case 26:
				/* produção vazia */
			break;
		case 27:
			empilhar(FATOR);
			break;
		case 28:
			empilhar(ID);
			break;
		case 29:
			empilhar(NUM);
			break;
		case 30:
			empilhar(FECHA_PAREN);
			empilhar(EXP);
			empilhar(ABRE_PAREN);
			break;
		default: printf("Erro: produção não implementada\n");
						break;
	}
}

// Função principal de análise sintática
void analisarSintaticamente(int tokens[], int num_tokens) {
int pos = 0;
empilhar(PROGRAMA); 

while (topo >= 0) {
    int topo_pilha = desempilhar();
    int token_atual = tokens[pos];

    // Terminal
    if (topo_pilha >= 0 && topo_pilha < NUM_TERMINAIS) {
        if (topo_pilha == token_atual) {
            printf("Match: %d\n", token_atual);
            pos++;
        } else {
            printf("Erro sintático: esperado %d, encontrado %d\n", topo_pilha, token_atual);
            return;
        }
    }
    // Não-terminal
    else if (topo_pilha >= 0 && topo_pilha < NUM_NAO_TERMINAIS) {
        int producao = tabelaM[topo_pilha][token_atual];
        if (producao != -1) {
            aplicarProducao(producao);
        } else {
            printf("Erro sintático: nenhuma produção para não-terminal %d com token %d\n", topo_pilha, token_atual);
            return;
        }
    }
}

if (pos == num_tokens) {
    printf("Análise sintática concluída com sucesso.\n");
} else {
    printf("Erro: tokens restantes após análise.\n");
}

// Exemplo de uso
int main() {
inicializarTabelaM();

int tokens_exemplo[] = {INT, ID, PONTO_VIRGULA}; // Ex: int x;
int num_tokens = sizeof(tokens_exemplo) / sizeof(tokens_exemplo[0]);

analisarSintaticamente(tokens_exemplo, num_tokens);
return 0;
}