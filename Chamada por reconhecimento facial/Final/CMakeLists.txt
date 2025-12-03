cmake_minimum_required(VERSION 3.10)
project(SistemaDeChamadaModular)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)
find_package(CURL REQUIRED)

include_directories(${OpenCV_INCLUDE_DIRS})

# 1. Cria o "Subsistema" (a biblioteca de reconhecimento)
add_library(reconhecimento_lib reconhecimento.cpp)
target_link_libraries(reconhecimento_lib ${OpenCV_LIBS})

# 2. Programa do Menu
add_executable(MenuPrincipal main.cpp)

# 3. Programa de Cadastro
add_executable(cadastrar_usuario cadastrar_usuario.cpp)
target_link_libraries(cadastrar_usuario reconhecimento_lib)

# 4. Programa da Chamada
add_executable(SistemaDeChamada SistemaDeChamada.cpp)
target_link_libraries(SistemaDeChamada reconhecimento_lib)

# 5. Presencas
add_executable(presencas Presencas.c)
target_link_libraries(SistemaDeChamada reconhecimento_lib)
target_link_libraries(presencas CURL::libcurl)