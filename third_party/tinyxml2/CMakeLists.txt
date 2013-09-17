add_definitions(-D_LIB)
add_library(tinyxml2 STATIC tinyxml2.cpp)

set(TINYXML_INCLUDEDIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(TINYXML_LIBRARIES tinyxml2 PARENT_SCOPE)

SET_PROPERTY(TARGET tinyxml2                PROPERTY FOLDER "ThirdParty")