#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <map>

using namespace std;
using namespace cv;
using namespace cv::face;

// Função para carregar o arquivo CSV com os nomes dos alunos
map<int, string> loadNames(const string& csv_path) {
    map<int, string> names;
    ifstream file(csv_path);
    string line;
    while (getline(file, line)) {
        stringstream ss(line);
        string id_str, name_str;
        if (getline(ss, id_str, ';') && getline(ss, name_str)) {
            try {
                names[stoi(id_str)] = name_str;
            } catch (const std::invalid_argument& e) {
                // Ignora linhas mal formatadas
            }
        }
    }
    return names;
}

int main(int argc, char** argv) {
    // --- 1. VERIFICAÇÃO DOS ARGUMENTOS ---
    if (argc != 2) {
        cout << "Uso: " << argv[0] << " <caminho_para_a_foto_de_teste.jpg>" << endl;
        return -1;
    }

    string test_image_path = argv[1];
    string cascade_path = "haarcascade_frontalface_default.xml";
    string model_path = "reconhecedor_facial.yml";
    string csv_path = "nomes.csv";

    // --- 2. CARREGAMENTO DO MODELO, NOMES E CLASSIFICADOR ---
    cout << "Carregando modelo de reconhecimento..." << endl;
    Ptr<LBPHFaceRecognizer> model = LBPHFaceRecognizer::create();
    model->read(model_path);

    cout << "Carregando banco de dados de nomes..." << endl;
    map<int, string> names = loadNames(csv_path);
    if (names.empty()) {
        cout << "Erro: Nenhum nome encontrado no arquivo CSV ou arquivo não existe." << endl;
        return -1;
    }

    CascadeClassifier face_cascade;
    if (!face_cascade.load(cascade_path)) {
        cout << "Erro: Não foi possível carregar o classificador Haar Cascade." << endl;
        return -1;
    }

    // --- 3. CARREGAMENTO E PROCESSAMENTO DA IMAGEM DE TESTE ---
    Mat test_image = imread(test_image_path, IMREAD_COLOR);
    if (test_image.empty()) {
        cout << "Erro: Não foi possível carregar a imagem de teste." << endl;
        return -1;
    }

    Mat gray;
    cvtColor(test_image, gray, COLOR_BGR2GRAY);
    
    vector<Rect> faces;
    face_cascade.detectMultiScale(gray, faces, 1.1, 5, 0|CASCADE_SCALE_IMAGE, Size(100, 100));

    // --- 4. DETECÇÃO E RECONHECIMENTO ---
    for (const auto& face : faces) {
        // Recorta e prepara o rosto para a predição (deve ser do mesmo tamanho usado no treino)
        Mat face_roi = gray(face);
        Mat resized_face;
        resize(face_roi, resized_face, Size(200, 200), 1.0, 1.0, INTER_CUBIC);

        int predicted_label = -1;
        double confidence = 0.0;
        model->predict(resized_face, predicted_label, confidence);

        string label_text;
        if (confidence < 80) { 
            label_text = names[predicted_label] + " (Conf: " + to_string(confidence) + ")";
        } else {
            label_text = "Desconhecido (Conf: " + to_string(confidence) + ")";
        }

        // Desenha o resultado na imagem
        rectangle(test_image, face, Scalar(0, 255, 0), 2);
        Point text_pos(face.x, face.y - 10);
        putText(test_image, label_text, text_pos, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 2);
    }
    
    // --- 5. EXIBIÇÃO DO RESULTADO ---
    imshow("Resultado do Reconhecimento", test_image);
    cout << "Pressione qualquer tecla na janela da imagem para sair." << endl;
    waitKey(0); 

    return 0;
}