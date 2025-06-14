# Construção de um Analisador Sintático 

## 🎯 Objetivo

Implementação de um **analisador sintático preditivo LL(1)**, em linguagem **C**, para verificar a **correção gramatical** de sequências de tokens geradas para uma **linguagem fictícia** inspirada em C/Java.

## 📝 Descrição

O analisador é capaz de identificar se uma sequência de tokens é **sintaticamente válida** com base em:

**Gramática Livre de Contexto (GLC)** ajustada para LL(1)  
**Tabela M** implementada manualmente  
Uso de **pilha** e **análise preditiva**  
**Tratamento completo de erros sintáticos**

## 🔧 Funcionalidades

Reconhecimento de declarações de variáveis e funções  
Estruturas de controle (if-else, while)  
Expressões aritméticas e de comparação  
Blocos de código com chaves  
Comandos de retorno (`return`)  
Chamadas de função com argumentos

## 🚀 Como Executar os Casos de Teste

Foram criados 10 casos de testes sintaticamente corretos e incorretos.
Para testa-los, no terminal, execute:

### Compilação
```
gcc analisadorSin.c -o analisadorSin -Wall
```
### Execução
```
./analisadorSin TokensX.txt
```
OBS: Substitua X pelo número ou nome do arquivo de teste, por exemplo: Tokens1.txt.

## Autoras
- Kailane Lisley de Araújo Silva
- Gabrielly Gouveia da Silva Feitosa

## Disciplina
Construção de Compiladores — Universidade Católica de Pernambuco (UNICAP), 2025.1
