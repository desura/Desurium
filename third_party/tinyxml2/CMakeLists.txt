add_definitions(-D_LIB)
add_library(tinyxml2 STATIC tinyxml2.cpp tinyxml2.h)

set(TINYXML_INCLUDEDIR ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)
set(TINYXML_LIBRARIES tinyxml2 PARENT_SCOPE)

set_property(TARGET tinyxml2 PROPERTY FOLDER "ThirdParty")
