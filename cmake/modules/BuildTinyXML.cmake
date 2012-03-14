set(TINYXML_GIT git://tinyxml.git.sourceforge.net/gitroot/tinyxml/tinyxml)

IF(NOT WIN32)
  message(FATAL_ERROR "please install tinyxml")
endif()

ExternalProject_Add(
    tinyxml
    GIT_REPOSITORY ${TINYXML_GIT}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
)

ExternalProject_Get_Property(
    tinyxml
    source_dir
)

set(TINYXML_INSTALL_DIR ${source_dir})

set(TINYXML_INCLUDE_DIRS ${TINYXML_INSTALL_DIR})
set(TINYXML_LIBRARIES ${TINYXML_INSTALL_DIR}/libtinyxml.a)

message("${CMAKE_CXX_CREATE_STATIC_LIBRARY}")
