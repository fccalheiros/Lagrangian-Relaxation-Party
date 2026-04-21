// Mestrado.cpp : Main function resides here
//

#include <iostream>
#include <string>
using namespace std;

#include "../ambiente/main.h"

int main(int argc, char* argv[])
{
#ifndef NDEBUG
    argc = 3;
    string str0 = "C:\\Users\\fccal\\iCloudDrive\\Notebook\\Documentos\\Pessoal\\mestrado\\instancias\\w50h50n30i10.pt";
    argv[1] = &str0[0];
    string str1 = "C:\\Users\\fccal\\iCloudDrive\\Notebook\\Documentos\\Pessoal\\mestrado\\instancias\\Configuration.json";
    argv[2] = &str1[0];
#endif
    try {
        return main2(argc, argv);
    }
    catch (exception &e) {
        cout << e.what() << endl;

    }

}


// Executar programa: Ctrl + F5 ou Menu Depurar > Iniciar Sem Depuração
// Depurar programa: F5 ou menu Depurar > Iniciar Depuração

// Dicas para Começar: 
//   1. Use a janela do Gerenciador de Soluções para adicionar/gerenciar arquivos
//   2. Use a janela do Team Explorer para conectar-se ao controle do código-fonte
//   3. Use a janela de Saída para ver mensagens de saída do build e outras mensagens
//   4. Use a janela Lista de Erros para exibir erros
//   5. Ir Para o Projeto > Adicionar Novo Item para criar novos arquivos de código, ou Projeto > Adicionar Item Existente para adicionar arquivos de código existentes ao projeto
//   6. No futuro, para abrir este projeto novamente, vá para Arquivo > Abrir > Projeto e selecione o arquivo. sln
