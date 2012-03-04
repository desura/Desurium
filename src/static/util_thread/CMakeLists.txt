include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GLOBAL_INCLUDE_DIRS}
  ${COMMON_INCLUDE_DIRS}
  ${TINYXML_INCLUDE_DIRS}
)

file(GLOB Sources code/*.cpp code/*.h)

add_library(threads STATIC ${Sources})
target_link_libraries(threads
    tinyxml
)
