cmake_minimum_required (VERSION 3.15)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

SET(ProjectName examples )

project(${ProjectName}  VERSION 1.0.0 LANGUAGES CXX)

 
 
set (NFEVAL_BASE_LIBS 
     ${NFEVAL_ROOT_DIR}/lib/NFBaseTypes${NFEVAL_VERSION}x64.lib
     ${NFEVAL_ROOT_DIR}/lib/NFEval${NFEVAL_VERSION}x64.lib
	 ${NFEVAL_ROOT_DIR}/lib/NFEvalLogger${NFEVAL_VERSION}x64.lib
	 ${NFEVAL_ROOT_DIR}/lib/NFComplexEvaluation${NFEVAL_VERSION}x64.lib
	 ${NFEVAL_ROOT_DIR}/lib/NFlog4cxx64.lib
)

set (NFEVAL_INCLUDE_DIR 
      ${NFEVAL_ROOT_DIR}/include
)

set( SRCS 
	main.cpp
	NFComplexEval.cpp 
    )
	
 
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /Od")   #disable optimization for RelWithDebugInfo  build configuration

 
add_executable(${ProjectName}  ${SRCS}   )

target_link_libraries(${ProjectName} PRIVATE  ${NFEVAL_BASE_LIBS})
target_compile_definitions(${ProjectName} PRIVATE  -DNOMINMAX )
target_include_directories(${ProjectName} PRIVATE ${NFEVAL_INCLUDE_DIR} )
 
set_target_properties(${ProjectName} PROPERTIES FOLDER "examples")

add_subdirectory(NFContour)
 
add_subdirectory(NFPrfIO)

if(USE_QT)

add_subdirectory(NFEvalGUI)

endif()