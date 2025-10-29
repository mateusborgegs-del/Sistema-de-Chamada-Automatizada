#include <opencv2/opencv.hpp>
#include <opencv2/face.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>

using namespace std;
using namespace cv;
using namespace cv::face;

// Função para ler o arquivo CSV e determinar o próximo ID disponível
int getNextId(const string& csv_path) {
    ifstream file(csv_path);
    string line;
    int max_id = 0;

    while (getline(file, line)) {
        stringstream ss(line);
        string id_str;
        // Pega a primeira parte da linha (o ID)
        getline(ss, id_str, ';');
        try {
            int id = stoi(id_str);
            if (id > max_id) {
                max_id = id;
            }
        } catch (const std::invalid_argument& e) {
            // Ignora linhas mal formatadas
        }
    }
    file.close();
    return max_id + 1;
}

int main(int argc, char** argv) {
   

    if (argc != 3) {
        cout << "Uso: " << argv[0] << " \"<Nome do Aluno>\" <caminho_para_a_foto.jpg>" << endl;
        return -1;
    }

    string student_name = argv[1];
    string image_path = argv[2];
    string cascade_path = "haarcascade_frontalface_default.xml";
    string model_path = "reconhecedor_facial.yml";
    string csv_path = "nomes.csv";



    Mat image = imread(image_path, IMREAD_COLOR);
    if (image.empty()) {
        cout << "Erro: Não foi possível carregar a imagem em: " << image_path << endl;
        return -1;
    }

    CascadeClassifier face_cascade;
    if (!face_cascade.load(cascade_path)) {
        cout << "Erro: Não foi possível carregar o classificador Haar Cascade." << endl;
        return -1;
    }

    Mat gray;
    cvtColor(image, gray, COLOR_BGR2GRAY);
    equalizeHist(gray, gray);

    vector<Rect> faces;
    face_cascade.detectMultiScale(gray, faces, 1.1, 5, 0|CASCADE_SCALE_IMAGE, Size(100, 100));

    if (faces.size() != 1) {
        cout << "Erro: A foto de cadastro deve conter exatamente um rosto. Foram encontrados: " << faces.size() << endl;
        return -1;
    }

    cout << "Rosto detectado com sucesso. Iniciando o processo de cadastro..." << endl;

  

    // Recorta o rosto da imagem original
    Mat face_roi = gray(faces[0]);

    // Padroniza o tamanho da imagem do rosto (importante para o reconhecimento)
    Mat resized_face;
    resize(face_roi, resized_face, Size(200, 200), 1.0, 1.0, INTER_CUBIC);

    // Determina o ID para o novo aluno
    int new_id = getNextId(csv_path);

    vector<Mat> images_to_train;
    vector<int> labels_to_train;

    images_to_train.push_back(resized_face);
    labels_to_train.push_back(new_id);
    
    // Cria o reconhecedor LBPH
    Ptr<LBPHFaceRecognizer> model = LBPHFaceRecognizer::create();

    // Se o modelo já existe, ele é carregado e atualizado (treinado com a nova foto).
    // Se não, um novo modelo é treinado do zero.
    try {
        model->read(model_path);
        cout << "Modelo existente carregado. Atualizando com o novo rosto..." << endl;
        model->update(images_to_train, labels_to_train);
    } catch (const cv::Exception& e) {
        cout << "Nenhum modelo encontrado. Criando um novo..." << endl;
        model->train(images_to_train, labels_to_train);
    }
    


    // Salva o modelo treinado/atualizado
    model->save(model_path);

    // Salva a associação ID -> Nome no arquivo CSV
    // O 'fstream::app' garante que a nova linha seja adicionada ao final do arquivo
    ofstream csv_file(csv_path, fstream::app);
    if (!csv_file.is_open()) {
        cout << "Erro ao abrir o arquivo CSV para escrita." << endl;
        return -1;
    }
    csv_file << new_id << ";" << student_name << endl;
    csv_file.close();

    cout << "------------------------------------------" << endl;
    cout << "Aluno cadastrado com sucesso!" << endl;
    cout << "ID: " << new_id << endl;
    cout << "Nome: " << student_name << endl;
    cout << "Modelo salvo em: " << model_path << endl;
    cout << "CSV atualizado em: " << csv_path << endl;
    cout << "------------------------------------------" << endl;

    return 0;
}