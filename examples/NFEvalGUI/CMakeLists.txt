

 
set(NFEvalGUI_SRCS
    NFEvalGUI.cpp
	NFEvalGUIMainWindow.cpp
    )

set(NFEvalGUI_HDRS
    NFEvalGUI.h
    )
	
set(NFEvalGUI_QT_HDRS
    NFEvalGUIMainWindow.h
    )


QT5_WRAP_CPP(NFEvalGUI_MOC_SRCS ${NFEvalGUI_QT_HDRS})
source_group("moc" FILES ${NFEvalGUI_MOC_SRCS} )


set(NFEvalGUI_UIS
    NFEvalGUI.ui
    )

QT5_WRAP_UI(NFEvalGUI_UIS_H ${NFEvalGUI_UIS})

 

add_executable(NFEvalGUI WIN32 ${NFEvalGUI_SRCS} ${NFEvalGUI_UIS_H} ${NFEvalGUI_MOC_SRCS} ${NFEvalGUI_HDRS} ${NFEvalGUI_QT_HDRS} )

target_link_libraries(NFEvalGUI PRIVATE ${NFEVAL_BASE_LIBS}
                                        ${NFEVAL_ROOT_DIR}/lib/NFQtWidgets${NFEVAL_VERSION}x64.lib
                                        Qt5::Widgets 
                                        Qt5::Core 
										Qt5::AxContainer
										Qt5::AxServer
										${NFEVAL_ROOT_DIR}/lib/vtk-8.2/vtkCommonCore-8.2.lib

										 
                     )
 
 
target_include_directories(NFEvalGUI  PRIVATE ${NFEVAL_INCLUDE_DIR}  
					  ${NFEVAL_ROOT_DIR}/include/vtk-8.2
					  ${CMAKE_CURRENT_BINARY_DIR}
                     )

target_compile_definitions(NFEvalGUI PRIVATE  -DNOMINMAX )


set_target_properties(NFEvalGUI PROPERTIES FOLDER "examples")



 

install(TARGETS NFEvalGUI RUNTIME DESTINATION bin)
