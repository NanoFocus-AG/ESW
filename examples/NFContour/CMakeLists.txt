cmake_minimum_required (VERSION 3.15)

SET( ProjectName NFContour )

project(${ProjectName}  VERSION 1.0.0 LANGUAGES CXX)

 
 

set (NFEVAL_BASE_LIBS 
     ${NFEVAL_ROOT_DIR}/lib/NFBaseTypes${NFEVAL_VERSION}x64.lib
     ${NFEVAL_ROOT_DIR}/lib/NFEval${NFEVAL_VERSION}x64.lib
	  ${NFEVAL_ROOT_DIR}/lib/NFEvalLogger${NFEVAL_VERSION}x64.lib
	   ${NFEVAL_ROOT_DIR}/lib/NFlog4cxx64.lib
)

set (NFEVAL_INCLUDE_DIR 
      ${NFEVAL_ROOT_DIR}/include
)

set(SRCS 
	NFContour.cpp
	NFContourMain.cpp
    )

add_library(${ProjectName} SHARED ${SRCS} )
target_link_libraries(${ProjectName}  ${NFEVAL_BASE_LIBS}  )
target_include_directories(${ProjectName} PRIVATE ${NFEVAL_INCLUDE_DIR} )
target_compile_definitions(${ProjectName} PRIVATE  -DNOMINMAX )
target_compile_definitions(${ProjectName} PRIVATE -DNFCONTUR_EXPORTS )

set_target_properties(${ProjectName} PROPERTIES FOLDER "examples/plugins")


set_target_properties(${ProjectName}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
   #    CXX_EXTENSIONS NO
)

#install dll to plugin folder
install(TARGETS  ${ProjectName} 
       RUNTIME DESTINATION ${NFEVAL_ROOT_DIR}/bin/Plugins
	   ARCHIVE DESTINATION ${NFEVAL_ROOT_DIR}/NFEval/lib
	) 

 