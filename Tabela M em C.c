#include <stdio.h>
#include <string.h>

#define NUM_NAO_TERMINAIS 21
#define NUM_TERMINAIS 42

// Enum para não-terminais
typedef enum {
    P, G, K, DF, COM, EC, B, E, EL, K1, K2, ER, EA, TM, F, T, PA, CL, CD, A, ARG
} NaoTerminal;

// Enum para terminais
typedef enum {
    INT, FLOAT, DOUBLE, CHAR, BOOLEAN, VOID, ID, ABRE_PAREN, FECHA_PAREN,
    ABRE_CHAVE, FECHA_CHAVE, PONTO_VIRGULA, ATRIBUICAO, IGUAL, DIFERENTE,
    MAIOR, MAIOR_IGUAL, MENOR, MENOR_IGUAL, MAIS, MENOS, MULT, DIV, MOD,
    AND, OR, IF, ELSE, WHILE, FOR, SWITCH, BREAK, CONTINUE, RETURN,
    CASE, DEFAULT, ABRE_COLCHETE, FECHA_COLCHETE, TEXTO, COMENTARIO, NUM, EPSILON
} Terminal;

// Tabela M: cada célula contém uma string com a produção
char* tabelaM[NUM_NAO_TERMINAIS][NUM_TERMINAIS];

void inicializarTabelaM() {
    // Limpa toda a tabela para NULL
    for(int i = 0; i < NUM_NAO_TERMINAIS; i++) {
        for(int j = 0; j < NUM_TERMINAIS; j++) {
            tabelaM[i][j] = NULL;
        }
    }

    // Programa → G P
    tabelaM[P][INT] = "P -> G P";
    tabelaM[P][FLOAT] = "P -> G P";
    tabelaM[P][DOUBLE] = "P -> G P";
    tabelaM[P][CHAR] = "P -> G P";
    tabelaM[P][BOOLEAN] = "P -> G P";
    tabelaM[P][VOID] = "P -> G P";
    tabelaM[P][COMENTARIO] = "P -> COM P";
    tabelaM[P][IF] = "P -> EC P";
    tabelaM[P][WHILE] = "P -> EC P";
    tabelaM[P][FOR] = "P -> EC P";
    tabelaM[P][SWITCH] = "P -> EC P";
    tabelaM[P][BREAK] = "P -> EC P";
    tabelaM[P][CONTINUE]= "P -> EC P";
    tabelaM[P][RETURN] = "P -> EC P";
    tabelaM[P][FECHA_CHAVE] = "P -> ε";

    // DeclaracaoVariavel → T ID = E ;
    tabelaM[G][INT] = "G -> T ID = E ;";
    tabelaM[G][FLOAT] = "G -> T ID = E ;";
    tabelaM[G][DOUBLE] = "G -> T ID = E ;";
    tabelaM[G][CHAR] = "G -> T ID = E ;";
    tabelaM[G][BOOLEAN] = "G -> T ID = E ;";

    // DeclaracaoFuncao → T ID ( PA ) B
    tabelaM[DF][VOID] = "DF -> T ID ( PA ) B";

    // Comentario
    tabelaM[COM][COMENTARIO] = "COM -> // texto";

    // Estrutura de Controle → if ( E ) B
    tabelaM[EC][IF] = "EC -> if ( E ) B";

    // Estrutura de Controle → while ( E ) B
    tabelaM[EC][WHILE] = "EC -> while ( E ) B";

    // Estrutura de Controle → for ( E ; E ; E ) B
    tabelaM[EC][FOR] = "EC -> for ( E ; E ; E ) B";

    // Estrutura de Controle → switch ( E ) CL
    tabelaM[EC][SWITCH] = "EC -> switch ( E ) CL";

    // Bloco → { P }
    tabelaM[B][ABRE_CHAVE] = "B -> { P }";

    // Expressão → ER
    tabelaM[E][ID] = "E -> ER";
    tabelaM[E][NUM] = "E -> ER";

    // Expressão Relação → EA K1 ER
    tabelaM[ER][ID] = "ER -> EA K1 ER";
    tabelaM[ER][NUM] = "ER -> EA K1 ER";

    // Termo Aditivo → TM EL
    tabelaM[EA][ID] = "EA -> TM EL";
    tabelaM[EA][NUM] = "EA -> TM EL";

    // Termo Multiplicativo → F TM2
    tabelaM[TM][ID] = "TM -> F TM2";
    tabelaM[TM][NUM] = "TM -> F TM2";

    // Fator → ID | NUM
    tabelaM[F][ID] = "F -> ID";
    tabelaM[F][NUM] = "F -> NUM";
}

int main() {
    inicializarTabelaM();
    printf("Tabela M inicializada com sucesso!\n");
    return 0;
}