include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${GLOBAL_INCLUDE_DIRS}
  ${COMMON_INCLUDE_DIRS}
  ${BZIP2_INCLUDE_DIR}
)

file(GLOB Headers code/*.h)
file(GLOB Sources code/*.cpp
)

add_library(utilcore SHARED ${Sources})
target_link_libraries(utilcore
    ${TINYXML_LIBRARIES}
    ${BZIP2_LIBRARIES}
)

install(TARGETS utilcore
        DESTINATION "${LIB_INSTALL_DIR}")
