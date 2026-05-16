// Mestrado.cpp : Main function resides here
//
#include <iostream>
#include <string>


#include "../ambiente/main.h"

int main(int argc, char* argv[])
{
#ifndef NDEBUG
    argc = 3;
    std::string path = "C:\\Users\\fccal\\iCloudDrive\\Notebook\\source\\repos\\Lagrangian-Relaxation-Party\\Instances\\";
    std::string str0 = path + "w50h50n36i16.pt";
    argv[1] = &str0[0];
    std::string str1 = path + "Configuration.json";
    argv[2] = &str1[0];
#endif

        try {
            return originalMain(argc, argv);
        }
        catch (std::exception& e) {
            std::cout << e.what() << std::endl << std::flush;

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
