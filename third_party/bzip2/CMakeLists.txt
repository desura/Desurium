include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(BZIP2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/ PARENT_SCOPE)

file(GLOB Headers ./include/*.h)
file(GLOB Sources ./code/*.c)

add_library(bzip2 STATIC ${Sources})
target_link_libraries(bzip2
)

set(BZIP2_LIBRARIES bzip2 PARENT_SCOPE)
