include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GLOBAL_INCLUDE_DIRS}
  ${COMMON_INCLUDE_DIRS}
  ${BREAKPAD_EXCEPTION_HANDLER_INCLUDE_DIR}
)

if(WIN32)
  file(GLOB Sources code/MiniDumpGenerator_win.cpp)
else()
  file(GLOB Sources code/MiniDumpGenerator_lin.cpp)
endif()

add_definitions(-fPIC)
add_library(mdump STATIC ${Sources})
target_link_libraries(mdump
    ${BREAKPAD_EXCEPTION_HANDLER_LIBRARIES}
    ${TINYXML_LIBRARIES}
)
