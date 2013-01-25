# from https://bitbucket.org/sumwars/sumwars-code/src/b2ba13b0d7406f9130df9f59a904c5fef69e29a3/CMakeModules/FindTinyXML.cmake?at=default

################################################################################
# Custom cmake module for CEGUI to find tinyxml
################################################################################
include(FindPackageHandleStandardArgs)
include(CheckCXXSourceCompiles)

find_path(TINYXML_H_PATH NAMES tinyxml.h)
find_library(TINYXML_LIB NAMES tinyxml libtinyxml)
mark_as_advanced(TINYXML_H_PATH TINYXML_LIB)

find_package_handle_standard_args(TINYXML DEFAULT_MSG TINYXML_LIB TINYXML_H_PATH)

if (TINYXML_FOUND)
    # what API version do we have here?
    set(CMAKE_REQUIRED_INCLUDES ${TINYXML_H_PATH})
    set(CMAKE_REQUIRED_LIBRARIES ${TINYXML_LIB})
    check_cxx_source_compiles("
    #include <tinyxml.h>
    int main() {
        int i = TiXmlElement::TINYXML_ELEMENT;
        return 0;
    }"
    
    TINYXML_API_TEST)

    set (TINYXML_HAS_2_6_API ${TINYXML_API_TEST})
    set (TINYXML_INCLUDE_DIR ${TINYXML_H_PATH})
    set (TINYXML_LIBRARIES ${TINYXML_LIB})
else()
    set (TINYXML_INCLUDE_DIR)
    set (TINYXML_LIBRARIES)
endif()

