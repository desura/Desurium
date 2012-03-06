include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(TINYXML_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/ PARENT_SCOPE)

file(GLOB Headers ./include/*.h)
file(GLOB Sources ./code/*.cpp ./include/*.h)

add_definitions(-fPIC)
add_library(tinyxml STATIC ${Sources})

set(TINYXML_LIBRARIES tinyxml PARENT_SCOPE)
