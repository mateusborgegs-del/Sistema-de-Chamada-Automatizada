cmake_minimum_required(VERSION 3.10)
project(ProjetoReconhecimentoFacial)

set(CMAKE_CXX_STANDARD 11)

find_package(OpenCV REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

# Compila o programa de chamada em tempo real
add_executable(SistemaDeChamada main.cpp)
target_link_libraries(SistemaDeChamada ${OpenCV_LIBS})

# Compila o novo programa de cadastro
add_executable(cadastrar_usuario cadastrar_usuario.cpp)
target_link_libraries(cadastrar_usuario ${OpenCV_LIBS})
# ... (linhas anteriores) ...

# Compila o novo programa de verificação
add_executable(verificar_reconhecimento verificar_reconhecimento.cpp)
target_link_libraries(verificar_reconhecimento ${OpenCV_LIBS})