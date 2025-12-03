#include "reconhecimento.h"
#include <iostream>
#include <set>
#include <fstream>

using namespace std;
using namespace cv;

void salvarPresentesCSV(const set<string> &presentes_hoje) {
    ofstream f("presentes.csv");

    if (!f.is_open()) {
        cerr << "Erro ao criar presentes.csv!" << endl;
        return;
    }

    // Cabeçalho compatível com o sistema semestral
    f << "Nome\n";

    // Salva cada nome
    for (const string &nome : presentes_hoje) {
        f << nome << "\n";
    }

    f.close();
    cout << "Arquivo presentes.csv gerado com sucesso!" << endl;
}


int main() {
    cout << "--- INICIANDO SISTEMA DE CHAMADA (MULTI-ARQUIVO) ---" << endl;
    
    // 1. Carrega o Classificador (XML)
    Ptr<CascadeClassifier> classificador = carregarClassificador();
    if (classificador == nullptr) {
        cout << "ERRO FATAL: Arquivo .xml (Haar Cascade) nao encontrado na pasta build." << endl;
        return -1;
    }

    // 2. Carrega TODOS os arquivos .yml da pasta modelos
    vector<Ptr<face::LBPHFaceRecognizer>> banco_modelos = carregarBancoDeModelos();
    
    if (banco_modelos.empty()) {
        cout << "ERRO: Nenhum modelo .yml encontrado na pasta 'modelos/'." << endl;
        cout << "Cadastre um aluno primeiro (Opcao 1)." << endl;
        return -1;
    }
    
    // 3. Carrega os Nomes (CSV)
    map<int, string> nomes = carregarNomes();
    if (nomes.empty()) {
        cout << "ERRO: Arquivo nomes.csv vazio ou inexistente." << endl;
        return -1;
    }

    // 4. Inicializa a Câmera
    VideoCapture cap;
    int camera_index = -1;
    for (int i = 0; i < 5; ++i) { 
        VideoCapture temp_cap(i, CAP_V4L2);
        if (temp_cap.isOpened()) { cap = temp_cap; camera_index = i; break; }
    }
    
    if (!cap.isOpened()) { 
        cout << "ERRO: Nenhuma câmera conectada encontrada." << endl; 
        return -1; 
    }

    cout << "Sistema pronto! Câmera índice: " << camera_index << endl;
    cout << "Pressione ESC para sair." << endl;
    
    set<string> presentes_hoje;
    Mat frame;

    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        vector<Rect> faces = detectarFaces(frame, classificador);
        Mat gray;
        cvtColor(frame, gray, COLOR_BGR2GRAY);

        for (const auto& face : faces) {
            Mat rosto = processarRosto(face, gray);
            
            // Usa a função que testa em todos os modelos carregados
            Predicao p = preverRostoMultiplo(banco_modelos, nomes, rosto);

            Scalar cor = Scalar(0, 0, 255); // Vermelho
            if (p.nome != "Desconhecido") {
                cor = Scalar(0, 255, 0); // Verde
                if (presentes_hoje.find(p.nome) == presentes_hoje.end()) {
                    presentes_hoje.insert(p.nome);
                    cout << "[PRESENCA REGISTRADA]: " << p.nome << " (Confianca: " << p.confianca << ")" << endl;
                }
            }

            rectangle(frame, face, cor, 2);
            putText(frame, p.nome, Point(face.x, face.y - 10), FONT_HERSHEY_SIMPLEX, 0.7, cor, 2);
        }

        imshow("Chamada", frame);
        if (waitKey(30) == 27) break;
    }

    cap.release();
    destroyAllWindows();
	
	salvarPresentesCSV(presentes_hoje);
	
    return 0;
}
