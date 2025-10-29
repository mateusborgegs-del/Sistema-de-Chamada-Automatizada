#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>


using namespace std;
using namespace cv;

int main() {
   

    // Abre a câmera padrão 
    VideoCapture cap(0);
    if (!cap.isOpened()) {
        cout << "Erro: Não foi possível abrir a câmera." << endl;
        return -1;
    }

    // Carrega o classificador Haar Cascade para detecção de faces frontais
    CascadeClassifier face_cascade;
    if (!face_cascade.load("haarcascade_frontalface_default.xml")) {
        cout << "Erro: Não foi possível carregar o arquivo do classificador haar cascade." << endl;
        return -1;
    }

    cout << "Sistema de Detecção Facial Iniciado." << endl;
    cout << "Pressione 'ESC' para sair." << endl;


  

    Mat frame;
    while (true) {
        // Captura um novo frame da câmera
        cap >> frame;
        if (frame.empty()) {
            cout << "Erro: Frame capturado está vazio." << endl;
            break;
        }

    

        // Converte o frame para escala de cinza (melhora o desempenho da detecção)
        Mat gray;
        cvtColor(frame, gray, COLOR_BGR2GRAY);

        // Melhora o contraste da imagem em escala de cinza para ajudar na detecção
        equalizeHist(gray, gray);

        // Vetor para armazenar as faces detectadas (cada face é um retângulo)
        vector<Rect> faces;

        // Detecta as faces na imagem
  
        face_cascade.detectMultiScale(gray, faces, 1.1, 3, 0|CASCADE_SCALE_IMAGE, Size(50, 50));



        // Desenha um retângulo ao redor de cada face detectada
        for (const auto& face : faces) {
            // Desenha um retângulo verde ao redor do rosto
            rectangle(frame, face, Scalar(0, 255, 0), 2);

            // Adiciona um texto acima do retângulo 
            string text = "Rosto Detectado";
            Point text_pos(face.x, face.y - 10);
            putText(frame, text, text_pos, FONT_HERSHEY_SIMPLEX, 0.5, Scalar(0, 255, 0), 2);
        }

        // Mostra o frame resultante em uma janela
        imshow("Sistema de Chamada - Detecção Facial", frame);

        // Espera por 10ms por uma tecla. Se for a tecla ESC (ASCII 27), sai do loop
        if (waitKey(10) == 27) {
            cout << "Encerrando o programa..." << endl;
            break;
        }
    }

    // Libera os recursos ao finalizar
    cap.release();
    destroyAllWindows();

    return 0;
}