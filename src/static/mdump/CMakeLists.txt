include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${BREAKPAD_EXCEPTION_HANDLER_INCLUDE_DIR}
)

if(WIN32)
  file(GLOB Sources code/MiniDumpGenerator_win.cpp)
  set(WINDOWS_LIBS Psapi)
else()
  file(GLOB Sources code/MiniDumpGenerator_lin.cpp)
endif()

add_library(mdump STATIC ${Sources})
target_link_libraries(mdump
    ${BREAKPAD_EXCEPTION_HANDLER_LIBRARIES}
	${WINDOWS_LIBS}
)

SET_PROPERTY(TARGET mdump                PROPERTY FOLDER "Static")
