cmake_minimum_required(VERSION 3.25)

project(Tests)



# Включаем функции GoogleTest

include(GoogleTest)
add_subdirectory(EngineTests)
add_subdirectory(nbstlTests)
