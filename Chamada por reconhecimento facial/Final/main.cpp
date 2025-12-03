
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <iostream>
#include <string>
#include <cstdlib> // Para system()

using namespace std;

// Função para exibir o menu principal
void showMenu() {
    cout << "\n--- Sistema de Chamada Automatizado ---" << endl;
    cout << "1. Cadastrar Novo Aluno" << endl;
    cout << "2. Iniciar Chamada (Abrir Câmera)" << endl;
    cout << "3. Sair" << endl;
    cout << "Escolha uma opção: ";
}

// Opção 1: Chama o executável de cadastro
void cadastrarAluno() {
    string nome;
    cout << "Digite o nome completo do aluno: ";
    cin.ignore(1000, '\n'); // Limpa o buffer do 'cin'
    getline(cin, nome);
    string command = "./cadastrar_usuario \"" + nome + "\"";
    cout << "Abrindo câmera para cadastro... Pressione 'ESPAÇO' para capturar." << endl;
    system(command.c_str());
    cout << "Processo de cadastro finalizado." << endl;
}

// Opção 2: Chama o executável da chamada
void iniciarChamada() {
    cout << "Iniciando sistema de chamada... Pressione 'ESC' na janela da câmera para sair." << endl;
    system("./SistemaDeChamada");
    cout << "Sistema de chamada encerrado." << endl;
    system("./presencas");
}

int main() {
    int choice = 0;
    while (choice != 3) {
        showMenu();
        cin >> choice;
        switch (choice) {
            case 1: cadastrarAluno(); break;
            case 2: iniciarChamada(); break;
            case 3: cout << "Encerrando o programa..." << endl; break;
            default:
                cout << "Opção inválida. Tente novamente." << endl;
                cin.clear();
                cin.ignore(1000, '\n');
                break;
        }

    }
    return 0;
}
