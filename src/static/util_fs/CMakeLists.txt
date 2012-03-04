find_package(Boost COMPONENTS filesystem)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GLOBAL_INCLUDE_DIRS}
  ${COMMON_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${TINYXML_INCLUDE_DIRS}
)
#  ${TINYXML_INCLUDE_DIRS}

file(GLOB Sources code/UtilFs.cpp code/*.h)

if(DEFINED WIN32)
  file(GLOB PlattFormSources code/UtilFs_win.cpp)
else()
  file(GLOB PlattFormSources code/UtilFs_nix.cpp)
endif()

add_library(util_fs SHARED ${Sources} ${PlattFormSources})
target_link_libraries(util_fs
    tinyxml
    ${Boost_LIBRARIES}
)
