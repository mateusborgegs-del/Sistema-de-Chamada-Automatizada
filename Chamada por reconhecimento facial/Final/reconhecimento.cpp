#include "reconhecimento.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <glob.h> 

using namespace std;
using namespace cv;
using namespace cv::face;

const string CASCADE_PATH = "haarcascade_frontalface_default.xml";
const string CSV_PATH = "nomes.csv";
const Size TAMANHO_PADRAO_ROSTO(200, 200);

// --- Funções Básicas ---
Ptr<CascadeClassifier> carregarClassificador() {
    Ptr<CascadeClassifier> classificador = makePtr<CascadeClassifier>(CASCADE_PATH);
    if (classificador->empty()) {
        cerr << "Erro: Classificador nao encontrado." << endl;
        return nullptr;
    }
    return classificador;
}

vector<Rect> detectarFaces(Mat& frame, Ptr<CascadeClassifier> classificador) {
    Mat gray;
    cvtColor(frame, gray, COLOR_BGR2GRAY);
    equalizeHist(gray, gray);
    vector<Rect> faces;
    classificador->detectMultiScale(gray, faces, 1.1, 5, 0|CASCADE_SCALE_IMAGE, Size(100, 100));
    return faces;
}

Mat processarRosto(Rect rosto, Mat frame_gray) {
    Mat face_roi = frame_gray(rosto);
    Mat resized_face;
    resize(face_roi, resized_face, TAMANHO_PADRAO_ROSTO, 1.0, 1.0, INTER_CUBIC);
    return resized_face;
}

map<int, string> carregarNomes() {
    map<int, string> names;
    ifstream file(CSV_PATH);
    string line;
    if (!file.is_open()) return names;
    while (getline(file, line)) {
        stringstream ss(line);
        string id_str, name_str;
        if (getline(ss, id_str, ',') && getline(ss, name_str)) {
            try { names[stoi(id_str)] = name_str; } catch (...) {}
        }
    }
    return names;
}

int getNextId() {
    map<int, string> names = carregarNomes();
    int max_id = 0;
    for (auto const& [id, nome] : names) {
        if (id > max_id) max_id = id;
    }
    return max_id + 1;
}

vector<Ptr<LBPHFaceRecognizer>> carregarBancoDeModelos() {
    vector<Ptr<LBPHFaceRecognizer>> banco_modelos;
    vector<String> arquivos;
    

    glob("modelos/*.yml", arquivos, false);

    for (const auto& arquivo : arquivos) {
        Ptr<LBPHFaceRecognizer> modelo = LBPHFaceRecognizer::create();
        try {
            modelo->read(arquivo);
            banco_modelos.push_back(modelo);
          
        } catch (...) {
            cerr << "Erro ao ler: " << arquivo << endl;
        }
    }
    
    if (banco_modelos.empty()) {
        cout << "[AVISO] Nenhum arquivo .yml encontrado na pasta 'modelos/'." << endl;
    } else {
        cout << "[SISTEMA] " << banco_modelos.size() << " modelos de alunos carregados na memoria." << endl;
    }

    return banco_modelos;
}


Predicao preverRostoMultiplo(const vector<Ptr<LBPHFaceRecognizer>>& banco_modelos, const map<int, string>& nomes, Mat rosto_processado) {
    Predicao melhor_predicao;
    melhor_predicao.nome = "Desconhecido";
    melhor_predicao.confianca = 1000.0;

    
    for (const auto& modelo : banco_modelos) {
        int label = -1;
        double conf = 0.0;
        
        modelo->predict(rosto_processado, label, conf);

    
        if (conf < melhor_predicao.confianca) {
            melhor_predicao.confianca = conf;
            if (conf < 75) { 
                if (nomes.count(label)) {
                    melhor_predicao.nome = nomes.at(label);
                } else {
                    melhor_predicao.nome = "ID " + to_string(label) + " (Sem Nome)";
                }
            } else {
                melhor_predicao.nome = "Desconhecido";
            }
        }
    }
    
    return melhor_predicao;
}


Ptr<LBPHFaceRecognizer> carregarModelo() { return nullptr; }
Predicao preverRosto(Ptr<LBPHFaceRecognizer> m, const map<int, string>& n, Mat r) { Predicao p; return p; }
void cadastrarNovoRosto(Ptr<LBPHFaceRecognizer> m, Mat r, int i, const string& n) {}
