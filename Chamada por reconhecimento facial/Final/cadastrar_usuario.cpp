#include "reconhecimento.h"
#include <iostream>
#include <fstream>
#include <sys/stat.h> // Para criar pastas

using namespace std;
using namespace cv;

int main(int argc, char** argv) {
    if (argc != 2) {
        cout << "Uso: " << argv[0] << " \"<Nome do Aluno>\"" << endl;
        return -1;
    }
    string student_name = argv[1];

    Ptr<CascadeClassifier> classificador = carregarClassificador();
    if (classificador == nullptr) return -1;

    // Inicializa Câmera
    VideoCapture cap;
    int camera_index = -1;
    for (int i = 0; i < 5; ++i) { 
        VideoCapture temp_cap(i, CAP_V4L2);
        if (temp_cap.isOpened()) { cap = temp_cap; camera_index = i; break; }
    }
    if (!cap.isOpened()) { cout << "Erro na camera." << endl; return -1; }

    // Cria a pasta 'modelos' se não existir
    system("mkdir -p modelos");

    // Variáveis de coleta
    vector<Mat> images_to_train;
    vector<int> labels_to_train;
    int amostras_meta = 30;
    int amostras_coletadas = 0;
    bool capturando = false;
    
    int novo_id = getNextId();

    cout << "\n--- CADASTRO INDIVIDUAL ---" << endl;
    cout << "Aluno: " << student_name << " (ID: " << novo_id << ")" << endl;
    cout << "Sera criado o arquivo: modelos/aluno_" << novo_id << ".yml" << endl;
    cout << "Pressione 'ESPAÇO' para iniciar." << endl;

    Mat frame;
    while (true) {
        cap >> frame;
        if (frame.empty()) break;
        
        Mat frame_display = frame.clone();
        vector<Rect> faces = detectarFaces(frame, classificador);

        for (const auto& face : faces) {
            Scalar color = capturando ? Scalar(0, 255, 255) : Scalar(0, 255, 0);
            rectangle(frame_display, face, color, 2);
        }
        
        if (capturando) {
            string msg = "Capturando: " + to_string(amostras_coletadas) + "/" + to_string(amostras_meta);
            putText(frame_display, msg, Point(20, 40), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);

            if (faces.size() == 1) {
                Mat gray;
                cvtColor(frame, gray, COLOR_BGR2GRAY);
                Mat rosto = processarRosto(faces[0], gray);
                
                images_to_train.push_back(rosto);
                labels_to_train.push_back(novo_id); // ID associado a esta imagem
                
                amostras_coletadas++;
                cout << "Amostra " << amostras_coletadas << "/" << amostras_meta << endl;
                waitKey(100); // Delay para variar a pose
            }
        } else {
            putText(frame_display, "ESPACO para iniciar", Point(20, 40), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 255, 0), 2);
        }

        imshow("Cadastro", frame_display);
        
        int key = waitKey(100);
        if (key == 27) return 0;
        if (key == 32 && !capturando) {
            if (!faces.empty()) capturando = true;
            else cout << "Rosto não detectado!" << endl;
        }

        if (amostras_coletadas >= amostras_meta) {
            cout << "Treinando modelo individual..." << endl;
            
            // Cria um reconhecedor NOVO e VAZIO apenas para este aluno
            Ptr<face::LBPHFaceRecognizer> modelo_aluno = face::LBPHFaceRecognizer::create();
            modelo_aluno->train(images_to_train, labels_to_train);
            
            // Salva com nome único
            string nome_arquivo = "modelos/aluno_" + to_string(novo_id) + ".yml";
            modelo_aluno->save(nome_arquivo);
            
            // Salva no CSV
            ofstream csv_file("nomes.csv", fstream::app);
            if (csv_file.is_open()) {
                csv_file << novo_id << "," << student_name << endl;
                csv_file.close();
            }
            
            cout << "Sucesso! Arquivo salvo: " << nome_arquivo << endl;
            break;
        }
    }
    
    cap.release();
    destroyAllWindows();
    return 0;
}