include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../mdump
  ${BREAKPAD_EXCEPTION_HANDLER_INCLUDE_DIR_S}
)

if(WIN32)
  file(GLOB Sources ${CMAKE_CURRENT_SOURCE_DIR}/../mdump/code/MiniDumpGenerator_win.cpp)
  set(WINDOWS_LIBS Psapi)
else()
  file(GLOB Sources ${CMAKE_CURRENT_SOURCE_DIR}/../mdump/code/MiniDumpGenerator_lin.cpp)
endif()

add_library(mdump_s STATIC ${Sources})
target_link_libraries(mdump_s
	${BREAKPAD_EXCEPTION_HANDLER_LIBRARIES_S}
	${WINDOWS_LIBS}
)

SET_PROPERTY(TARGET mdump_s                PROPERTY FOLDER "Static")
SetStaticRuntime(mdump_s)
