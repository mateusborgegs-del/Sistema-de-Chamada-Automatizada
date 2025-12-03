#ifndef RECONHECIMENTO_H
#define RECONHECIMENTO_H

#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <string>
#include <vector>
#include <map>

// --- DEFINIÇÃO DA ESTRUTURA (Deve vir antes das funções) ---
struct Predicao {
    std::string nome;
    double confianca;
};

// --- Declaração das Funções ---

cv::Ptr<cv::CascadeClassifier> carregarClassificador();

std::vector<cv::Rect> detectarFaces(cv::Mat& frame, cv::Ptr<cv::CascadeClassifier> classificador);

cv::Mat processarRosto(cv::Rect rosto, cv::Mat frame_gray);

std::map<int, std::string> carregarNomes();

int getNextId();

// Carrega Múltiplos Modelos
std::vector<cv::Ptr<cv::face::LBPHFaceRecognizer>> carregarBancoDeModelos();

// Faz a predição testando em todos os modelos
Predicao preverRostoMultiplo(
    const std::vector<cv::Ptr<cv::face::LBPHFaceRecognizer>>& banco_modelos, 
    const std::map<int, std::string>& nomes, 
    cv::Mat rosto_processado
);

// Mantidas para compatibilidade (se necessário)
cv::Ptr<cv::face::LBPHFaceRecognizer> carregarModelo();
Predicao preverRosto(cv::Ptr<cv::face::LBPHFaceRecognizer> modelo, const std::map<int, std::string>& nomes, cv::Mat rosto_processado);
void cadastrarNovoRosto(cv::Ptr<cv::face::LBPHFaceRecognizer> modelo, cv::Mat rosto_processado, int novo_id, const std::string& nome_aluno);

#endif // RECONHECIMENTO_H