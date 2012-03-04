include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GLOBAL_INCLUDE_DIRS}
  ${COMMON_INCLUDE_DIRS}
  ${TINYXML_INCLUDE_DIRS}
)

file(GLOB Sources code/*.cpp)

add_definitions(-fPIC)
add_library(umcf STATIC ${Sources})
target_link_libraries(umcf
    tinyxml
)
