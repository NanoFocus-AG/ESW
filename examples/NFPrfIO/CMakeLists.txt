
add_definitions(-DNFPrfIO_EXPORTS) 

 

set( NFPrfIOName
NFPrfIO${NFEVAL_DLL_POSTFIX}
)

set(NFPrfIO_SRCS 
	NFPrfIOMain.cpp
	NFPrfIO.cpp
    )
	
set(NFPrfIO_HDRS
	NFPrfIO_Config.h
	NFPrfIO.h
	)
	


	 

#Build shared library (dll) 
add_library(${NFPrfIOName} SHARED ${NFPrfIO_SRCS} ${NFPrfIO_HDRS}  )
target_link_libraries(${NFPrfIOName}  ${NFEVAL_BASE_LIBS}  )
target_include_directories(${NFPrfIOName} PRIVATE ${NFEVAL_INCLUDE_DIR} )
target_compile_definitions(${NFPrfIOName} PRIVATE  -DNOMINMAX )

 
 
set_target_properties(${NFPrfIOName} PROPERTIES FOLDER "examples/plugins")


set_target_properties(${NFPrfIOName}
    PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED YES
        CXX_EXTENSIONS NO
)

#install dll to plugin folder
install(TARGETS  ${NFPrfIOName} 
       RUNTIME DESTINATION bin/Plugins
	   ARCHIVE DESTINATION NFEval/lib
	)
#install include Files
install(FILES ${NFPrfIO_HDRS} DESTINATION NFEval/include)
 