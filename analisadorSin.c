#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_PILHA 500 // Increased stack size
#define MAX_TOKENS 1000 // Increased token buffer size
#define MAX_TAM_TOKEN 50

// Estrutura para armazenar informações do token lido do arquivo
typedef struct {
    char nome[MAX_TAM_TOKEN]; // Nome do token (ex: T_INT, T_ID)
    char lexema[MAX_TAM_TOKEN]; // O lexema real (ex: "int", "x")
    char tipo_lexema[MAX_TAM_TOKEN]; // Categoria (ex: PALAVRA_RESERVADA)
} TokenInfoArquivo;

// === Enum para Terminais (Baseado em gramatica_padronizada.txt e Tabela M) ===
// Ordem deve ser consistente com as colunas da Tabela M
enum Terminais {
    INT, FLOAT, CHAR, BOOLEAN, VOID, ID, NUM,
    IF, WHILE, ELSE, RETURN, 
    COMP, // Operadores de comparação (==, !=, <, >, <=, >=) 
    OP_ARIT, // Operadores aritméticos (+, -, *, /)
    ABRE_PAREN, FECHA_PAREN, ABRE_CHAVE, FECHA_CHAVE,
    PONTO_VIRGULA, VIRGULA, ATRIBUICAO,
    T_EOF,
    // --- Fim dos terminais usados na Tabela M --- //
    NUM_TERMINAIS // Contador - deve ser o último
};

// === Enum para Não-Terminais (Baseado em gramatica_padronizada.txt e Tabela M) ===
// Ordem deve ser consistente com as linhas da Tabela M
enum NaoTerminais {
    Programa,
    Declaracoes,
    Declaracao,
    Tipo,
    DeclaracaoResto,
    Parametros,
    ListaParametros,
    RestoListaParametros,
    Bloco,
    Comandos,
    Comando,
    ComandoCondicional,
    ElseOpcional,
    ExpressaoOpcional,
    Expressao,
    ExpressaoResto,
    Termo,
    TermoResto,
    Fator, // Usando a regra modificada Fator -> T_ID FatorResto | T_NUM | ...
    FatorResto, // Novo não-terminal para a modificação LL(1)
    Argumentos,
    ListaArgumentos,
    RestoListaArgumentos,
    // --- Fim dos não-terminais usados na Tabela M --- //
    NUM_NAO_TERMINAIS // Contador - deve ser o último
};

// Define um offset para os não-terminais na pilha
#define NT_DESLOCAMENTO NUM_TERMINAIS
// Macros para converter entre enum base e valor na pilha
#define NT_PILHA(nt_base) (nt_base + NT_DESLOCAMENTO)
#define PILHA_NT(pilha_valor) (pilha_valor - NT_DESLOCAMENTO)

// === Pilha ===
int pilha[MAX_PILHA];
int topo = -1;

void empilhar(int simbolo) {
    if (topo < MAX_PILHA - 1) {
        pilha[++topo] = simbolo;
    } else {
        fprintf(stderr, "Erro: Estouro da pilha!\n");
        exit(EXIT_FAILURE); // Termina se a pilha estourar
    }
}

int desempilhar() {
    if (topo >= 0) {
        return pilha[topo--];
    } else {
        // Não é necessariamente um erro, pode ser o fim da análise
        // fprintf(stderr, "Alerta: Tentativa de desempilhar com a pilha vazia!\n");
        return -1; // Retorna um valor inválido
    }
}

// === Tabela M ===
// Dimensões baseadas nos enums acima
int tabelaM[NUM_NAO_TERMINAIS][NUM_TERMINAIS];

// === Números das Produções (conforme definido no plano) ===
// 0. Programa' → Programa T_EOF (Implícito)
#define P_Programa_Declaracoes 1
#define P_Declaracoes_DeclaracaoDeclaracoes 2
#define P_Declaracoes_Epsilon 3
#define P_Declaracao_TipoIdDeclaracaoResto 4
#define P_DeclaracaoResto_PontoVirgula 5
#define P_DeclaracaoResto_AtribuicaoExprPontoVirgula 6
#define P_DeclaracaoResto_ParamsBloco 7
#define P_Parametros_ListaParametros 8
#define P_Parametros_Epsilon 9
#define P_ListaParametros_TipoIdRestoListaParametros 10
#define P_RestoListaParametros_VirgulaTipoIdResto 11
#define P_RestoListaParametros_Epsilon 12
#define P_Tipo_Int 13
#define P_Tipo_Float 14
#define P_Tipo_Char 15
#define P_Tipo_Boolean 16
#define P_Tipo_Void 17
#define P_Bloco_AbreChaveComandosFechaChave 18
#define P_Comandos_ComandoComandos 19
#define P_Comandos_Epsilon 20
#define P_Comando_Declaracao 21
#define P_Comando_IdAtribExprPontoVirgula 22
#define P_Comando_ComandoCondicional 23
#define P_Comando_Bloco 24
#define P_Comando_ReturnExprOpcPontoVirgula 25
#define P_ComandoCondicional_If 26
#define P_ComandoCondicional_While 27
#define P_ElseOpcional_ElseComando 28
#define P_ElseOpcional_Epsilon 29
#define P_ExpressaoOpcional_Expressao 30
#define P_ExpressaoOpcional_Epsilon 31
#define P_Expressao_TermoExpressaoResto 32
#define P_ExpressaoResto_CompTermoExpressaoResto 33
#define P_ExpressaoResto_Epsilon 34
#define P_Termo_FatorTermoResto 35
#define P_TermoResto_OpAritFatorTermoResto 36
#define P_TermoResto_Epsilon 37
#define P_Fator_IdFatorResto 38 // Regra Modificada
#define P_Fator_Num 39
#define P_Fator_AbreParenExprFechaParen 40
#define P_FatorResto_Call 41 // Regra Modificada
#define P_FatorResto_Epsilon 42 // Regra Modificada
#define P_Argumentos_ListaArgumentos 43
#define P_Argumentos_Epsilon 44
#define P_ListaArgumentos_ExprRestoListaArgumentos 45
#define P_RestoListaArgumentos_VirgulaExprResto 46
#define P_RestoListaArgumentos_Epsilon 47

// Função para inicializar a Tabela M
void inicializarTabelaM() {
    // 1. Inicializa toda a tabela com -1 (indicando erro/sem produção)
    for (int i = 0; i < NUM_NAO_TERMINAIS; i++) {
        for (int j = 0; j < NUM_TERMINAIS; j++) {
            tabelaM[i][j] = -1;
        }
    }

    // 2. Preenche as entradas válidas com o número da produção correspondente
    //    Baseado em "Tabela M Completa (Gramática Padronizada e Simplificada).md"

    // Programa
    tabelaM[Programa][INT] = P_Programa_Declaracoes; // 1
    tabelaM[Programa][FLOAT] = P_Programa_Declaracoes; // 1
    tabelaM[Programa][CHAR] = P_Programa_Declaracoes; // 1
    tabelaM[Programa][BOOLEAN] = P_Programa_Declaracoes; // 1
    tabelaM[Programa][VOID] = P_Programa_Declaracoes; // 1
    tabelaM[Programa][T_EOF] = P_Programa_Declaracoes; // 1 (Programa -> Declaracoes, e Declaracoes pode ser epsilon)

    // Declaracoes
    tabelaM[Declaracoes][INT] = P_Declaracoes_DeclaracaoDeclaracoes; // 2
    tabelaM[Declaracoes][FLOAT] = P_Declaracoes_DeclaracaoDeclaracoes; // 2
    tabelaM[Declaracoes][CHAR] = P_Declaracoes_DeclaracaoDeclaracoes; // 2
    tabelaM[Declaracoes][BOOLEAN] = P_Declaracoes_DeclaracaoDeclaracoes; // 2
    tabelaM[Declaracoes][VOID] = P_Declaracoes_DeclaracaoDeclaracoes; // 2
    // FOLLOW(Declaracoes) inclui T_EOF e T_FECHA_CHAVE (porque pode vir antes de '}')
    tabelaM[Declaracoes][T_EOF] = P_Declaracoes_Epsilon; // 3
    tabelaM[Declaracoes][FECHA_CHAVE] = P_Declaracoes_Epsilon; // 3

    // Declaracao
    tabelaM[Declaracao][INT] = P_Declaracao_TipoIdDeclaracaoResto; // 4
    tabelaM[Declaracao][FLOAT] = P_Declaracao_TipoIdDeclaracaoResto; // 4
    tabelaM[Declaracao][CHAR] = P_Declaracao_TipoIdDeclaracaoResto; // 4
    tabelaM[Declaracao][BOOLEAN] = P_Declaracao_TipoIdDeclaracaoResto; // 4
    tabelaM[Declaracao][VOID] = P_Declaracao_TipoIdDeclaracaoResto; // 4

    // Tipo
    tabelaM[Tipo][INT] = P_Tipo_Int; // 13
    tabelaM[Tipo][FLOAT] = P_Tipo_Float; // 14
    tabelaM[Tipo][CHAR] = P_Tipo_Char; // 15
    tabelaM[Tipo][BOOLEAN] = P_Tipo_Boolean; // 16
    tabelaM[Tipo][VOID] = P_Tipo_Void; // 17

    // DeclaracaoResto
    tabelaM[DeclaracaoResto][PONTO_VIRGULA] = P_DeclaracaoResto_PontoVirgula; // 5 (VAR)
    tabelaM[DeclaracaoResto][ATRIBUICAO] = P_DeclaracaoResto_AtribuicaoExprPontoVirgula; // 6 (VAR_ASSIGN)
    tabelaM[DeclaracaoResto][ABRE_PAREN] = P_DeclaracaoResto_ParamsBloco; // 7 (FUNC)

    // Parametros
    tabelaM[Parametros][INT] = P_Parametros_ListaParametros; // 8 (LST)
    tabelaM[Parametros][FLOAT] = P_Parametros_ListaParametros; // 8
    tabelaM[Parametros][CHAR] = P_Parametros_ListaParametros; // 8
    tabelaM[Parametros][BOOLEAN] = P_Parametros_ListaParametros; // 8
    tabelaM[Parametros][VOID] = P_Parametros_ListaParametros; // 8
    tabelaM[Parametros][FECHA_PAREN] = P_Parametros_Epsilon; // 9 (ε)

    // ListaParametros
    tabelaM[ListaParametros][INT] = P_ListaParametros_TipoIdRestoListaParametros; // 10
    tabelaM[ListaParametros][FLOAT] = P_ListaParametros_TipoIdRestoListaParametros; // 10
    tabelaM[ListaParametros][CHAR] = P_ListaParametros_TipoIdRestoListaParametros; // 10
    tabelaM[ListaParametros][BOOLEAN] = P_ListaParametros_TipoIdRestoListaParametros; // 10
    tabelaM[ListaParametros][VOID] = P_ListaParametros_TipoIdRestoListaParametros; // 10

    // RestoListaParametros
    tabelaM[RestoListaParametros][VIRGULA] = P_RestoListaParametros_VirgulaTipoIdResto; // 11 (,)
    tabelaM[RestoListaParametros][FECHA_PAREN] = P_RestoListaParametros_Epsilon; // 12 (ε)

    // Bloco
    tabelaM[Bloco][ABRE_CHAVE] = P_Bloco_AbreChaveComandosFechaChave; // 18

    // Comandos
    // FIRST(Comando) = FIRST(Declaracao) U {T_ID, T_IF, T_WHILE, T_RETURN, T_ABRE_CHAVE}
    tabelaM[Comandos][INT] = P_Comandos_ComandoComandos; // 19
    tabelaM[Comandos][FLOAT] = P_Comandos_ComandoComandos; // 19
    tabelaM[Comandos][CHAR] = P_Comandos_ComandoComandos; // 19
    tabelaM[Comandos][BOOLEAN] = P_Comandos_ComandoComandos; // 19
    tabelaM[Comandos][VOID] = P_Comandos_ComandoComandos; // 19
    tabelaM[Comandos][ID] = P_Comandos_ComandoComandos; // 19
    tabelaM[Comandos][IF] = P_Comandos_ComandoComandos; // 19
    tabelaM[Comandos][WHILE] = P_Comandos_ComandoComandos; // 19
    tabelaM[Comandos][RETURN] = P_Comandos_ComandoComandos; // 19
    tabelaM[Comandos][ABRE_CHAVE] = P_Comandos_ComandoComandos; // 19
    // FOLLOW(Comandos) = {T_FECHA_CHAVE}
    tabelaM[Comandos][FECHA_CHAVE] = P_Comandos_Epsilon; // 20 (ε)

    // Comando
    tabelaM[Comando][INT] = P_Comando_Declaracao; // 21 (DEC)
    tabelaM[Comando][FLOAT] = P_Comando_Declaracao; // 21
    tabelaM[Comando][CHAR] = P_Comando_Declaracao; // 21
    tabelaM[Comando][BOOLEAN] = P_Comando_Declaracao; // 21
    tabelaM[Comando][VOID] = P_Comando_Declaracao; // 21
    tabelaM[Comando][ID] = P_Comando_IdAtribExprPontoVirgula; // 22 (ASSIGN)
    tabelaM[Comando][IF] = P_Comando_ComandoCondicional; // 23 (COND)
    tabelaM[Comando][WHILE] = P_Comando_ComandoCondicional; // 23
    tabelaM[Comando][RETURN] = P_Comando_ReturnExprOpcPontoVirgula; // 25 (RETURN)
    tabelaM[Comando][ABRE_CHAVE] = P_Comando_Bloco; // 24 (BLOCO)

    // ComandoCondicional
    tabelaM[ComandoCondicional][IF] = P_ComandoCondicional_If; // 26 (IF)
    tabelaM[ComandoCondicional][WHILE] = P_ComandoCondicional_While; // 27 (WHILE)

    // ElseOpcional
    tabelaM[ElseOpcional][ELSE] = P_ElseOpcional_ElseComando; // 28 (ELSE)
    // FOLLOW(ElseOpcional) = FOLLOW(Comando) = FIRST(Comando) U FOLLOW(Comandos) U {T_ELSE}
    // Simplificando: Tudo que pode vir depois de um comando.
    tabelaM[ElseOpcional][INT] = P_ElseOpcional_Epsilon; // 29 (ε)
    tabelaM[ElseOpcional][FLOAT] = P_ElseOpcional_Epsilon; // 29
    tabelaM[ElseOpcional][CHAR] = P_ElseOpcional_Epsilon; // 29
    tabelaM[ElseOpcional][BOOLEAN] = P_ElseOpcional_Epsilon; // 29
    tabelaM[ElseOpcional][VOID] = P_ElseOpcional_Epsilon; // 29
    tabelaM[ElseOpcional][ID] = P_ElseOpcional_Epsilon; // 29
    tabelaM[ElseOpcional][IF] = P_ElseOpcional_Epsilon; // 29
    tabelaM[ElseOpcional][WHILE] = P_ElseOpcional_Epsilon; // 29
    tabelaM[ElseOpcional][RETURN] = P_ElseOpcional_Epsilon; // 29
    tabelaM[ElseOpcional][ABRE_CHAVE] = P_ElseOpcional_Epsilon; // 29
    tabelaM[ElseOpcional][FECHA_CHAVE] = P_ElseOpcional_Epsilon; // 29 (do FOLLOW(Comandos))
    tabelaM[ElseOpcional][EOF] = P_ElseOpcional_Epsilon; // 29 (do FOLLOW(Declaracoes))

    // ExpressaoOpcional
    // FIRST(Expressao) = FIRST(Termo) = FIRST(Fator) = {T_ID, T_NUM, T_ABRE_PAREN}
    tabelaM[ExpressaoOpcional][ID] = P_ExpressaoOpcional_Expressao; // 30 (E)
    tabelaM[ExpressaoOpcional][NUM] = P_ExpressaoOpcional_Expressao; // 30
    tabelaM[ExpressaoOpcional][ABRE_PAREN] = P_ExpressaoOpcional_Expressao; // 30
    // FOLLOW(ExpressaoOpcional) = {T_PONTO_VIRGULA}
    tabelaM[ExpressaoOpcional][PONTO_VIRGULA] = P_ExpressaoOpcional_Epsilon; // 31 (ε)

    // Expressao
    tabelaM[Expressao][ID] = P_Expressao_TermoExpressaoResto; // 32
    tabelaM[Expressao][NUM] = P_Expressao_TermoExpressaoResto; // 32
    tabelaM[Expressao][ABRE_PAREN] = P_Expressao_TermoExpressaoResto; // 32

    // ExpressaoResto
    tabelaM[ExpressaoResto][COMP] = P_ExpressaoResto_CompTermoExpressaoResto; // 33 (COMP)
    // FOLLOW(ExpressaoResto) = FOLLOW(Expressao) = {T_FECHA_PAREN, T_PONTO_VIRGULA, T_VIRGULA}
    tabelaM[ExpressaoResto][FECHA_PAREN] = P_ExpressaoResto_Epsilon; // 34 (ε)
    tabelaM[ExpressaoResto][PONTO_VIRGULA] = P_ExpressaoResto_Epsilon; // 34
    tabelaM[ExpressaoResto][VIRGULA] = P_ExpressaoResto_Epsilon; // 34

    // Termo
    tabelaM[Termo][ID] = P_Termo_FatorTermoResto; // 35
    tabelaM[Termo][NUM] = P_Termo_FatorTermoResto; // 35
    tabelaM[Termo][ABRE_PAREN] = P_Termo_FatorTermoResto; // 35

    // TermoResto
    tabelaM[TermoResto][OP_ARIT] = P_TermoResto_OpAritFatorTermoResto; // 36 (OP)
    // FOLLOW(TermoResto) = FOLLOW(Termo) = FOLLOW(Expressao) U FIRST(ExpressaoResto) = {T_FECHA_PAREN, T_PONTO_VIRGULA, T_VIRGULA, T_COMP}
    tabelaM[TermoResto][FECHA_PAREN] = P_TermoResto_Epsilon; // 37 (ε)
    tabelaM[TermoResto][PONTO_VIRGULA] = P_TermoResto_Epsilon; // 37
    tabelaM[TermoResto][VIRGULA] = P_TermoResto_Epsilon; // 37
    tabelaM[TermoResto][COMP] = P_TermoResto_Epsilon; // 37

    // Fator (Regra Modificada)
    tabelaM[Fator][ID] = P_Fator_IdFatorResto; // 38 (ID)
    tabelaM[Fator][NUM] = P_Fator_Num; // 39 (NUM)
    tabelaM[Fator][ABRE_PAREN] = P_Fator_AbreParenExprFechaParen; // 40 (PAREN)

    // FatorResto (Regra Modificada)
    tabelaM[FatorResto][ABRE_PAREN] = P_FatorResto_Call; // 41 (CALL)
    // FOLLOW(FatorResto) = FOLLOW(Fator) = FOLLOW(Termo) U FIRST(TermoResto) = {T_FECHA_PAREN, T_PONTO_VIRGULA, T_VIRGULA, T_COMP, T_OP_ARIT}
    tabelaM[FatorResto][FECHA_PAREN] = P_FatorResto_Epsilon; // 42 (ε)
    tabelaM[FatorResto][PONTO_VIRGULA] = P_FatorResto_Epsilon; // 42
    tabelaM[FatorResto][VIRGULA] = P_FatorResto_Epsilon; // 42
    tabelaM[FatorResto][COMP] = P_FatorResto_Epsilon; // 42
    tabelaM[FatorResto][OP_ARIT] = P_FatorResto_Epsilon; // 42

    // Argumentos
    // FIRST(ListaArgumentos) = FIRST(Expressao) = {T_ID, T_NUM, T_ABRE_PAREN}
    tabelaM[Argumentos][ID] = P_Argumentos_ListaArgumentos; // 43 (LST)
    tabelaM[Argumentos][NUM] = P_Argumentos_ListaArgumentos; // 43
    tabelaM[Argumentos][ABRE_PAREN] = P_Argumentos_ListaArgumentos; // 43
    // FOLLOW(Argumentos) = {T_FECHA_PAREN}
    tabelaM[Argumentos][FECHA_PAREN] = P_Argumentos_Epsilon; // 44 (ε)

    // ListaArgumentos
    tabelaM[ListaArgumentos][ID] = P_ListaArgumentos_ExprRestoListaArgumentos; // 45
    tabelaM[ListaArgumentos][NUM] = P_ListaArgumentos_ExprRestoListaArgumentos; // 45
    tabelaM[ListaArgumentos][ABRE_PAREN] = P_ListaArgumentos_ExprRestoListaArgumentos; // 45

    // RestoListaArgumentos
    tabelaM[RestoListaArgumentos][VIRGULA] = P_RestoListaArgumentos_VirgulaExprResto; // 46 (,)
    // FOLLOW(RestoListaArgumentos) = FOLLOW(Argumentos) = {T_FECHA_PAREN}
    tabelaM[RestoListaArgumentos][FECHA_PAREN] = P_RestoListaArgumentos_Epsilon; // 47 (ε)

    printf("Tabela M inicializada completamente.\n");
}

// Função para aplicar uma produção da gramática
void aplicarProducao(int producao) {
    // Empilha os símbolos do lado direito da produção em ordem inversa
    switch (producao) {
        case P_Programa_Declaracoes: // 1: Programa → Declaracoes
            empilhar(NT_PILHA(Declaracoes));
            break;
        case P_Declaracoes_DeclaracaoDeclaracoes: // 2: Declaracoes → Declaracao Declaracoes
            empilhar(NT_PILHA(Declaracoes));
            empilhar(NT_PILHA(Declaracao));
            break;
        case P_Declaracoes_Epsilon: // 3: Declaracoes → ε
            // Não empilha nada
            break;
        case P_Declaracao_TipoIdDeclaracaoResto: // 4: Declaracao → Tipo T_ID DeclaracaoResto
            empilhar(NT_PILHA(DeclaracaoResto));
            empilhar(ID);
            empilhar(NT_PILHA(Tipo));
            break;
        case P_DeclaracaoResto_PontoVirgula: // 5: DeclaracaoResto → T_PONTO_VIRGULA
            empilhar(PONTO_VIRGULA);
            break;
        case P_DeclaracaoResto_AtribuicaoExprPontoVirgula: // 6: DeclaracaoResto → T_ATRIBUICAO Expressao T_PONTO_VIRGULA
            empilhar(PONTO_VIRGULA);
            empilhar(NT_PILHA(Expressao));
            empilhar(ATRIBUICAO);
            break;
        case P_DeclaracaoResto_ParamsBloco: // 7: DeclaracaoResto → T_ABRE_PAREN Parametros T_FECHA_PAREN Bloco
            empilhar(NT_PILHA(Bloco));
            empilhar(FECHA_PAREN);
            empilhar(NT_PILHA(Parametros));
            empilhar(ABRE_PAREN);
            break;
        case P_Parametros_ListaParametros: // 8: Parametros → ListaParametros
            empilhar(NT_PILHA(ListaParametros));
            break;
        case P_Parametros_Epsilon: // 9: Parametros → ε
            // Não empilha nada
            break;
        case P_ListaParametros_TipoIdRestoListaParametros: // 10: ListaParametros → Tipo T_ID RestoListaParametros
            empilhar(NT_PILHA(RestoListaParametros));
            empilhar(ID);
            empilhar(NT_PILHA(Tipo));
            break;
        case P_RestoListaParametros_VirgulaTipoIdResto: // 11: RestoListaParametros → T_VIRGULA Tipo T_ID RestoListaParametros
            empilhar(NT_PILHA(RestoListaParametros));
            empilhar(ID);
            empilhar(NT_PILHA(Tipo));
            empilhar(VIRGULA);
            break;
        case P_RestoListaParametros_Epsilon: // 12: RestoListaParametros → ε
            // Não empilha nada
            break;
        case P_Tipo_Int: empilhar(INT); break; // 13
        case P_Tipo_Float: empilhar(FLOAT); break; // 14
        case P_Tipo_Char: empilhar(CHAR); break; // 15
        case P_Tipo_Boolean: empilhar(BOOLEAN); break; // 16
        case P_Tipo_Void: empilhar(VOID); break; // 17
        case P_Bloco_AbreChaveComandosFechaChave: // 18: Bloco → T_ABRE_CHAVE Comandos T_FECHA_CHAVE
            empilhar(FECHA_CHAVE);
            empilhar(NT_PILHA(Comandos));
            empilhar(ABRE_CHAVE);
            break;
        case P_Comandos_ComandoComandos: // 19: Comandos → Comando Comandos
            empilhar(NT_PILHA(Comandos));
            empilhar(NT_PILHA(Comando));
            break;
        case P_Comandos_Epsilon: // 20: Comandos → ε
            // Não empilha nada
            break;
        case P_Comando_Declaracao: // 21: Comando → Declaracao
            empilhar(NT_PILHA(Declaracao));
            break;
        case P_Comando_IdAtribExprPontoVirgula: // 22: Comando → T_ID T_ATRIBUICAO Expressao T_PONTO_VIRGULA
            empilhar(PONTO_VIRGULA);
            empilhar(NT_PILHA(Expressao));
            empilhar(ATRIBUICAO);
            empilhar(ID);
            break;
        case P_Comando_ComandoCondicional: // 23: Comando → ComandoCondicional
            empilhar(NT_PILHA(ComandoCondicional));
            break;
        case P_Comando_Bloco: // 24: Comando → Bloco
            empilhar(NT_PILHA(Bloco));
            break;
        case P_Comando_ReturnExprOpcPontoVirgula: // 25: Comando → T_RETURN ExpressaoOpcional T_PONTO_VIRGULA
            empilhar(PONTO_VIRGULA);
            empilhar(NT_PILHA(ExpressaoOpcional));
            empilhar(RETURN);
            break;
        case P_ComandoCondicional_If: // 26: ComandoCondicional → T_IF T_ABRE_PAREN Expressao T_FECHA_PAREN Comando ElseOpcional
            empilhar(NT_PILHA(ElseOpcional));
            empilhar(NT_PILHA(Comando));
            empilhar(FECHA_PAREN);
            empilhar(NT_PILHA(Expressao));
            empilhar(ABRE_PAREN);
            empilhar(IF);
            break;
        case P_ComandoCondicional_While: // 27: ComandoCondicional → T_WHILE T_ABRE_PAREN Expressao T_FECHA_PAREN Comando
            empilhar(NT_PILHA(Comando));
            empilhar(FECHA_PAREN);
            empilhar(NT_PILHA(Expressao));
            empilhar(ABRE_PAREN);
            empilhar(WHILE);
            break;
        case P_ElseOpcional_ElseComando: // 28: ElseOpcional → T_ELSE Comando
            empilhar(NT_PILHA(Comando));
            empilhar(ELSE);
            break;
        case P_ElseOpcional_Epsilon: // 29: ElseOpcional → ε
            // Não empilha nada
            break;
        case P_ExpressaoOpcional_Expressao: // 30: ExpressaoOpcional → Expressao
            empilhar(NT_PILHA(Expressao));
            break;
        case P_ExpressaoOpcional_Epsilon: // 31: ExpressaoOpcional → ε
            // Não empilha nada
            break;
        case P_Expressao_TermoExpressaoResto: // 32: Expressao → Termo ExpressaoResto
            empilhar(NT_PILHA(ExpressaoResto));
            empilhar(NT_PILHA(Termo));
            break;
        case P_ExpressaoResto_CompTermoExpressaoResto: // 33: ExpressaoResto → T_COMP Termo ExpressaoResto
            empilhar(NT_PILHA(ExpressaoResto));
            empilhar(NT_PILHA(Termo));
            empilhar(COMP);
            break;
        case P_ExpressaoResto_Epsilon: // 34: ExpressaoResto → ε
            // Não empilha nada
            break;
        case P_Termo_FatorTermoResto: // 35: Termo → Fator TermoResto
            empilhar(NT_PILHA(TermoResto));
            empilhar(NT_PILHA(Fator));
            break;
        case P_TermoResto_OpAritFatorTermoResto: // 36: TermoResto → T_OP_ARIT Fator TermoResto
            empilhar(NT_PILHA(TermoResto));
            empilhar(NT_PILHA(Fator));
            empilhar(OP_ARIT);
            break;
        case P_TermoResto_Epsilon: // 37: TermoResto → ε
            // Não empilha nada
            break;
        case P_Fator_IdFatorResto: // 38: Fator → T_ID FatorResto (Modificada)
            empilhar(NT_PILHA(FatorResto));
            empilhar(ID);
            break;
        case P_Fator_Num: // 39: Fator → T_NUM
            empilhar(NUM);
            break;
        case P_Fator_AbreParenExprFechaParen: // 40: Fator → T_ABRE_PAREN Expressao T_FECHA_PAREN
            empilhar(FECHA_PAREN);
            empilhar(NT_PILHA(Expressao));
            empilhar(ABRE_PAREN);
            break;
        case P_FatorResto_Call: // 41: FatorResto → T_ABRE_PAREN Argumentos T_FECHA_PAREN (Modificada)
            empilhar(FECHA_PAREN);
            empilhar(NT_PILHA(Argumentos));
            empilhar(ABRE_PAREN);
            break;
        case P_FatorResto_Epsilon: // 42: FatorResto → ε (Modificada)
            // Não empilha nada
            break;
        case P_Argumentos_ListaArgumentos: // 43: Argumentos → ListaArgumentos
            empilhar(NT_PILHA(ListaArgumentos));
            break;
        case P_Argumentos_Epsilon: // 44: Argumentos → ε
            // Não empilha nada
            break;
        case P_ListaArgumentos_ExprRestoListaArgumentos: // 45: ListaArgumentos → Expressao RestoListaArgumentos
            empilhar(NT_PILHA(RestoListaArgumentos));
            empilhar(NT_PILHA(Expressao));
            break;
        case P_RestoListaArgumentos_VirgulaExprResto: // 46: RestoListaArgumentos → T_VIRGULA Expressao RestoListaArgumentos
            empilhar(NT_PILHA(RestoListaArgumentos));
            empilhar(NT_PILHA(Expressao));
            empilhar(VIRGULA);
            break;
        case P_RestoListaArgumentos_Epsilon: // 47: RestoListaArgumentos → ε
            // Não empilha nada
            break;

        default:
            fprintf(stderr, "Erro Interno: Tentativa de aplicar produção não implementada ou invalida: %d\n", producao);
            exit(EXIT_FAILURE);
            break;
    }
}

// Função para mapear nome do token (string) para enum
// **PRECISA SER COMPLETADO CUIDADOSAMENTE**
int mapearToken(const char* nomeToken) {
    // Palavras-chave e Tipos
    if (strcmp(nomeToken, "INT") == 0 || strcmp(nomeToken, "int") == 0) return INT;
    if (strcmp(nomeToken, "FLOAT") == 0 || strcmp(nomeToken, "float") == 0) return FLOAT;
    if (strcmp(nomeToken, "CHAR") == 0 || strcmp(nomeToken, "char") == 0) return CHAR;
    if (strcmp(nomeToken, "BOOLEAN") == 0 || strcmp(nomeToken, "boolean") == 0) return BOOLEAN;
    if (strcmp(nomeToken, "VOID") == 0 || strcmp(nomeToken, "void") == 0) return VOID;
    if (strcmp(nomeToken, "IF") == 0 || strcmp(nomeToken, "if") == 0) return IF;
    if (strcmp(nomeToken, "ELSE") == 0 || strcmp(nomeToken, "else") == 0) return ELSE;
    if (strcmp(nomeToken, "WHILE") == 0 || strcmp(nomeToken, "while") == 0) return WHILE;
    if (strcmp(nomeToken, "RETURN") == 0 || strcmp(nomeToken, "return") == 0) return RETURN;

    // Identificadores e Números
    if (strcmp(nomeToken, "ID") == 0 || strcmp(nomeToken, "ID") == 0) return ID;
    if (strcmp(nomeToken, "NUM") == 0 || strcmp(nomeToken, "NUM_INT") == 0 || strcmp(nomeToken, "NUM_DEC") == 0) return NUM;

    // Operadores de Comparação (Todos mapeados para T_COMP por simplicidade na Tabela M)
    // Se sua Tabela M diferenciar, você precisará de enums/colunas separadas
    if (strcmp(nomeToken, "COMP") == 0 || strcmp(nomeToken, "==") == 0 || strcmp(nomeToken, "!=") == 0 ||
        strcmp(nomeToken, "<") == 0 || strcmp(nomeToken, ">") == 0 || strcmp(nomeToken, "<=") == 0 ||
        strcmp(nomeToken, ">=") == 0) return COMP;

    // Operadores Aritméticos (Todos mapeados para T_OP_ARIT por simplicidade na Tabela M)
    // Se sua Tabela M diferenciar, você precisará de enums/colunas separadas
    if (strcmp(nomeToken, "OP_ARIT") == 0 || strcmp(nomeToken, "+") == 0 || strcmp(nomeToken, "-") == 0 ||
        strcmp(nomeToken, "*") == 0 || strcmp(nomeToken, "/") == 0 || strcmp(nomeToken, "%") == 0)
        return OP_ARIT;

    // Símbolos Especiais
    if (strcmp(nomeToken, "ABRE_PAREN") == 0 || strcmp(nomeToken, "(") == 0) return ABRE_PAREN;
    if (strcmp(nomeToken, "FECHA_PAREN") == 0 || strcmp(nomeToken, ")") == 0) return FECHA_PAREN;
    if (strcmp(nomeToken, "ABRE_CHAVE") == 0 || strcmp(nomeToken, "{") == 0) return ABRE_CHAVE;
    if (strcmp(nomeToken, "FECHA_CHAVE") == 0 || strcmp(nomeToken, "}") == 0) return FECHA_CHAVE;
    if (strcmp(nomeToken, "PONTO_VIRGULA") == 0 || strcmp(nomeToken, ";") == 0) return PONTO_VIRGULA;
    if (strcmp(nomeToken, "VIRGULA") == 0 || strcmp(nomeToken, ",") == 0) return VIRGULA;
    if (strcmp(nomeToken, "ATRIBUICAO") == 0 || strcmp(nomeToken, "=") == 0) return ATRIBUICAO;

    // Fim de Arquivo
    if (strcmp(nomeToken, "T_EOF") == 0 || strcmp(nomeToken, "T_EOF") == 0 || strcmp(nomeToken, "$") == 0) return T_EOF;

    // Adicione outros mapeamentos se necessário (ex: T_TEXTO, T_COMENTARIO, se forem relevantes para a sintaxe)

    // Se o token lido não for reconhecido
    fprintf(stderr, "Alerta: Token desconhecido no arquivo de entrada: %s\n", nomeToken);
    return -1; // Retorna um valor inválido
}

// Função auxiliar para imprimir nome do símbolo (debug)
const char* nomeSimbolo(int simbolo) {
    // Terminais
    if (simbolo >= 0 && simbolo < NUM_TERMINAIS) {
        switch ((enum Terminais)simbolo) {
            case INT: return "INT";
            case FLOAT: return "FLOAT"; 
            case CHAR: return "CHAR";
            case BOOLEAN: return "BOOLEAN";
            case VOID: return "VOID";
            case ID: return "ID";
            case NUM: return "NUM"; 
            case IF: return "IF"; 
            case WHILE: return "WHILE";
            case RETURN: return "RETURN"; 
            case COMP: return "COMP"; 
            case OP_ARIT: return "OP_ARIT";
            case ABRE_PAREN: return "ABRE_PAREN"; 
            case FECHA_PAREN: return "FECHA_PAREN";
            case ABRE_CHAVE: return "ABRE_CHAVE"; 
            case FECHA_CHAVE: return "FECHA_CHAVE";
            case PONTO_VIRGULA: return "PONTO_VIRGULA"; 
            case VIRGULA: return "VIRGULA";
            case ATRIBUICAO: return "ATRIBUICAO"; 
            case ELSE: return "ELSE"; 
            case T_EOF: return "T_EOF";
            default: return "TERMINAL_DESCONHECIDO";
        }
    }
    // Não-Terminais
    else if (simbolo >= NT_DESLOCAMENTO && simbolo < (NT_DESLOCAMENTO + NUM_NAO_TERMINAIS)) {
        int nt_base = PILHA_NT(simbolo);
         switch ((enum NaoTerminais)nt_base) {
            case Programa: return "Programa";
            case Declaracoes: return "Declaracoes";
            case Declaracao: return "Declaracao";
            case Tipo: return "Tipo"; 
            case DeclaracaoResto: return "DeclaracaoResto";
            case Parametros: return "Parametros";
            case ListaParametros: return "ListaParametros"; 
            case RestoListaParametros: return "RestoListaParametros";
            case Bloco: return "Bloco"; 
            case Comandos: return "Comandos"; 
            case Comando: return "Comando";
            case ComandoCondicional: return "ComandoCondicional"; 
            case ElseOpcional: return "ElseOpcional";
            case ExpressaoOpcional: return "ExpressaoOpcional"; 
            case Expressao: return "Expressao";
            case ExpressaoResto: return "ExpressaoResto"; 
            case Termo: return "Termo"; 
            case TermoResto: return "TermoResto";
            case Fator: return "Fator"; 
            case FatorResto: return "FatorResto"; 
            case Argumentos: return "Argumentos";
            case ListaArgumentos: return "ListaArgumentos"; 
            case RestoListaArgumentos: return "RestoListaArgumentos";
            default: return "NAO_TERMINAL_DESCONHECIDO";
        }
    }
    return "SIMBOLO_INVALIDO";
}

int main(int argc, char *argv[]) {
    int debug = 0; 
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo_tokens> [-debug]\n", argv[0]);
        return 1;
    }
    if (argc > 2 && strcmp(argv[2], "debug") == 0) {
        debug = 1;
        printf("Modo DEBUG ativado.\n");
    }

    inicializarTabelaM();
    topo = -1; // Garante que a pilha está vazia
    empilhar(T_EOF); // Empilha marcador de fim de entrada primeiro
    empilhar(NT_PILHA(Programa)); // Empilha símbolo inicial da gramática

    if(debug) printf("Pilha inicializada. Símbolo inicial: %s, Marcador EOF: %s\n", nomeSimbolo(NT_PILHA(Programa)), nomeSimbolo(T_EOF));
    // --- Leitura dos Tokens do Arquivo ---
    TokenInfoArquivo tokensArquivo[MAX_TOKENS];
    int tokensEnum[MAX_TOKENS];
    int totalTokens = 0;

    FILE* arq = fopen(argv[1], "r");
    if (!arq) {
        perror("Erro ao abrir o arquivo de tokens");
        return 1;
    }

    if(debug) printf("Lendo tokens de %s...\n", argv[1]);
    // Lê as 3 colunas, mas usa principalmente a primeira para mapear
    while (totalTokens < MAX_TOKENS && fscanf(arq, "%49s %*s %*s", tokensArquivo[totalTokens].nome) == 1) {
        strcpy(tokensArquivo[totalTokens].lexema, tokensArquivo[totalTokens].nome);
        strcpy(tokensArquivo[totalTokens].tipo_lexema, "N/A");
        tokensEnum[totalTokens] = mapearToken(tokensArquivo[totalTokens].nome);
        if (tokensEnum[totalTokens] == -1) {
            fprintf(stderr, "Erro: Token '%s' (lexema: '%s') não reconhecido na linha %d (aprox.) do arquivo.\n",
                    tokensArquivo[totalTokens].nome, tokensArquivo[totalTokens].lexema, totalTokens + 1);
            fclose(arq);
            return 1; // Aborta se encontrar token desconhecido
        }
        if(debug) printf("  Lido token %d: %s -> %s\n", totalTokens, tokensArquivo[totalTokens].nome, nomeSimbolo(tokensEnum[totalTokens]));
        totalTokens++;
    }
    fclose(arq);

    // Adiciona T_EOF ao final da sequência de tokens lida
    if (totalTokens < MAX_TOKENS) {
        tokensEnum[totalTokens] = T_EOF;
        strcpy(tokensArquivo[totalTokens].nome, "T_EOF"); // Para mensagens de erro
        strcpy(tokensArquivo[totalTokens].lexema, "$");
        totalTokens++; // Incrementa para incluir o T_EOF na contagem
    } else {
        fprintf(stderr, "Erro: Número máximo de tokens (%d) excedido.\n", MAX_TOKENS);
        return 1;
    }

    printf("%d tokens lidos e mapeados (incluindo T_EOF).\n", totalTokens);

    // --- Loop Principal da Análise Sintática ---
    int posTokenAtual = 0;
    int simboloAtual = tokensEnum[posTokenAtual];

    printf("\nIniciando analise sintatica...\n");

    while (topo >= 0) { // Enquanto a pilha não estiver vazia
        int simboloTopo = pilha[topo]; // Apenas espia o topo

        if (debug) printf("Topo da pilha: %s (%d), Token atual: %s (%d)\n",
               nomeSimbolo(simboloTopo), simboloTopo, tokensArquivo[posTokenAtual].nome, simboloAtual);

        // Verifica se o símbolo no topo é um terminal
        if (simboloTopo >= 0 && simboloTopo < NUM_TERMINAIS) {
            desempilhar(); // Remove o terminal da pilha ANTES de comparar
            if (simboloTopo == simboloAtual) {
                if (simboloTopo == T_EOF) {
                     if(debug) printf("  Match! Terminal: %s. Fim da análise esperado.\n", nomeSimbolo(simboloTopo));
                     // Se o EOF foi encontrado e era esperado, a análise deve terminar.
                     // A verificação final (pilha vazia) acontece fora do loop.
                     posTokenAtual++; // Consome o EOF da entrada
                     break; // Sai do loop principal
                } else {
                    if(debug) printf("  Match! Terminal: %s\n", nomeSimbolo(simboloTopo));
                    posTokenAtual++; // Avança para o próximo token de entrada
                    if (posTokenAtual < totalTokens) {
                        simboloAtual = tokensEnum[posTokenAtual];
                    } else {
                        fprintf(stderr, "Erro: Fim inesperado da entrada após match do token %s.\n", nomeSimbolo(simboloTopo));
                        goto erro_sintatico; // Fim prematuro da entrada
                    }
                }
            } else {
                fprintf(stderr, "\nErro Sintático: Terminal esperado '%s' (%d) não corresponde ao token atual '%s' (%d).\n", nomeSimbolo(simboloTopo), simboloTopo, tokensArquivo[posTokenAtual].nome, simboloAtual);
                fprintf(stderr, "\\nContexto: Lexema '%s' na linha %d (aprox.)\n", tokensArquivo[posTokenAtual].lexema, posTokenAtual + 1);
                goto erro_sintatico; // Pula para o final com erro
            }
        }
        // Verifica se o símbolo no topo é um não-terminal
        else if (simboloTopo >= NT_DESLOCAMENTO && simboloTopo < (NT_DESLOCAMENTO + NUM_NAO_TERMINAIS)) {
            int nt_base = PILHA_NT(simboloTopo); 
            // Consulta a Tabela M
            int producao = -1;
            if (simboloAtual >= 0 && simboloAtual < NUM_TERMINAIS) { // Garante que o token atual é válido
                 producao = tabelaM[nt_base][simboloAtual];
            }

            if (debug) printf("  Consulta Tabela M: TabelaM[%s][%s] = %d\n",
                   nomeSimbolo(simboloTopo), nomeSimbolo(simboloAtual), producao);

            if (producao != -1) {
                // Produção encontrada, aplica
                if (debug) printf("  Aplicando produção %d para não-terminal %s\n", producao, nomeSimbolo(simboloTopo));
                desempilhar(); // Remove o não-terminal do topo ANTES de empilhar a produção
                aplicarProducao(producao);
                if (debug) {
                    printf("  Pilha após aplicar produção: ");
                    for(int i=0; i<=topo; ++i) printf("%s ", nomeSimbolo(pilha[i]));
                    printf("\n");
                }
            } else {
                // Erro: Nenhuma produção válida na Tabela M
                fprintf(stderr, "\nErro Sintático: Nenhuma produção na Tabela M para não-terminal '%s' (%d) com token atual '%s' (%d).\n",
                        nomeSimbolo(simboloTopo), simboloTopo, tokensArquivo[posTokenAtual].nome, simboloAtual);
                fprintf(stderr, "             Contexto: Lexema '%s' na linha %d (aprox.)\n", tokensArquivo[posTokenAtual].lexema, posTokenAtual + 1);
                goto erro_sintatico; // Pula para o final com erro
            }
        } else {
            // Erro: Símbolo inválido na pilha (não deveria acontecer)
            fprintf(stderr, "\nErro Interno: Símbolo inválido %d encontrado no topo da pilha (não é terminal nem não-terminal conhecido).\n", simboloTopo);
            goto erro_sintatico; // Pula para o final com erro
        }
    } // Fim do while(topo >= 0)

    // --- Verificação Final ---
    // A análise é bem-sucedida se a pilha estiver vazia E o último token consumido foi EOF
    if (topo < 0 && posTokenAtual == totalTokens) { // posTokenAtual aponta para DEPOIS do EOF lido
         printf("\nAnalise sintatica concluída com sucesso!\n\n");
         return 0; // Sucesso
    } else {
        fprintf(stderr, "\nErro: Análise sintática falhou. Estado final inválido.\n");
        if (topo >= 0) {
            fprintf(stderr, "  Pilha não está vazia. Topo: %s (%d).\n", nomeSimbolo(pilha[topo]), pilha[topo]);
        } else if (posTokenAtual < totalTokens) {
             fprintf(stderr, "  Tokens restantes na entrada a partir de '%s'.\n", tokensArquivo[posTokenAtual].nome);
        } else {
             fprintf(stderr, "  Condição de parada inesperada (topo=%d, posTokenAtual=%d, totalTokens=%d).\n", topo, posTokenAtual, totalTokens);
        }
        goto erro_sintatico;
    }

erro_sintatico:
    fprintf(stderr, "Análise sintática encerrada com erros.\n");
    return 1; // Falha
}