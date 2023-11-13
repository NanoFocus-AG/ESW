CMAKE_MINIMUM_REQUIRED(VERSION 3.15)

project(ESW VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


set(CMAKE_PREFIX_PATH "M:/Win64_VS2017/qt-5.12.4") # Without this line, cmake does not find the package

find_package(Qt5 COMPONENTS Core Widgets REQUIRED  )

set(CMAKE_AUTOMOC ON) # For meta object compiler
set(CMAKE_AUTORCC ON) # Resource files
set(CMAKE_AUTOUIC ON) # UI files