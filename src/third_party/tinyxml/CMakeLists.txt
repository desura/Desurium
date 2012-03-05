include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(TINYXML_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include/ PARENT_SCOPE)

file(GLOB TinyXMLHeaders ./include/*.h)
file(GLOB TinyXMLSources ./code/*.cpp ./include/*.h)

add_definitions(-fPIC)
add_library(tinyxml STATIC ${TinyXMLSources})
