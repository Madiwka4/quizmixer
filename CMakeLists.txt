cmake_minimum_required(VERSION 3.14)
project(MadExam)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(MadExam
    main.cpp
    logic/quiz.cpp
    logic/quiz.h
    logic/docwriter.cpp
    logic/docwriter.h
    MainWindow.cpp
    MainWindow.h
    cli.cpp
    cli.h
)

target_include_directories(MadExam PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(MadExam PRIVATE
    Qt6::Widgets
)