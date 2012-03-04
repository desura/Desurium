find_package(GoogleBreakpad REQUIRED)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GLOBAL_INCLUDE_DIRS}
  ${COMMON_INCLUDE_DIRS}
  ${BREAKPAD_EXCEPTION_HANDLER_INCLUDE_DIR}
  ${TINYXML_INCLUDE_DIRS}
)

if(DEFINED WIN32)
  file(GLOB Sources code/MiniDumpGenerator_win.cpp)
else()
  file(GLOB Sources code/MiniDumpGenerator_lin.cpp)
endif()

add_library(mdump SHARED ${Sources})
target_link_libraries(mdump
    tinyxml
    ${BREAKPAD_EXCEPTION_HANDLER_LIBRARIES}
)
