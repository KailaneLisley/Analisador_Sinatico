@echo off
echo Compilando...
gcc Analisador_Sintatico.c -o Analisador_Sintatico

echo Iniciando testes automatizados...

for %%f in (Tokens*.txt) do (
    echo ----------------------------
    echo Executando teste com %%f
    analisador %%f
)

echo ----------------------------
echo Todos os testes foram executados.
pause