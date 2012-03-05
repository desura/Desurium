include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${GLOBAL_INCLUDE_DIRS}
  ${COMMON_INCLUDE_DIRS}
  ${BZIP2_INCLUDE_DIR}
  ${TINYXML_INCLUDE_DIRS}
)

file(GLOB Headers code/*.h)
file(GLOB Sources code/*.cpp
)

if(DEFINED WIN32)
  file(GLOB PlattformSources)
else()
  file(GLOB PlattformSources)
endif()

add_library(utilcore SHARED ${Sources} ${PlattformSources})
target_link_libraries(utilcore
    tinyxml
    ${BZIP2_LIBRARIES}
)

install(TARGETS utilcore
        DESTINATION "${LIB_INSTALL_DIR}")
