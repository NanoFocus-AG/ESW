CMAKE_MINIMUM_REQUIRED(VERSION 3.15)

project(ESW VERSION 1.0)


find_path(NFEVAL_ROOT_DIR "") 
set (NFEVAL_VERSION 85) 


option(USE_QT "" OFF)
  
if(USE_QT)

 set(CMAKE_PREFIX_PATH "M:/Win64_VS2017/qt-5.12.4") # Without this line, cmake does not find the package

 find_package(Qt5 COMPONENTS Core Widgets REQUIRED  )

 set(CMAKE_AUTOMOC ON) # For meta object compiler

 set(CMAKE_AUTORCC ON) # Resource files 

 set(CMAKE_AUTOUIC ON) # UI files

endif()


add_subdirectory(examples)