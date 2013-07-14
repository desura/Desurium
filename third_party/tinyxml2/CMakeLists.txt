add_definitions(-D_LIB)
add_library(tinyxml2 STATIC tinyxml2.cpp)

set(TINYXML_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/ PARENT_SCOPE)
set(TINYXML_LIBRARIES tinyxml2 PARENT_SCOPE)
