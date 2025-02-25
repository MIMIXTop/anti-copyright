# Минимальная требуемая версия CMake
cmake_minimum_required(VERSION 3.10)

# Название проекта
project(GloVe)

# Установка стандарта C++ (например, C++17)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Поиск пакета Boost
find_package(Boost 1.82.0 REQUIRED COMPONENTS system)

# Если Boost не найден, CMake выдаст ошибку
if (NOT Boost_FOUND)
    message(FATAL_ERROR "Boost not found!")
endif()

# Добавление исполняемого файла
add_executable(boost_example main.cpp)

# Линковка с Boost
target_link_libraries(boost_example PRIVATE Boost::system)