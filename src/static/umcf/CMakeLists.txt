include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${BZIP2_INCLUDE_DIR}
)

file(GLOB Sources code/*.cpp)

add_library(umcf STATIC ${Sources})
target_link_libraries(umcf
    ${BZIP2_LIBRARIES}
)
