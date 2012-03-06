include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${GLOBAL_INCLUDE_DIRS}
  ${COMMON_INCLUDE_DIRS}
  ${SQLITE3X_INCLUDE_DIRS}
)

file(GLOB Headers code/*.h)
file(GLOB Sources code/*.cpp
)

if(WIN32)
  file(GLOB PlattformSources)
else()
  file(GLOB PlattformSources)
endif()

add_library(webcore SHARED ${Sources} ${PlattformSources})
target_link_libraries(webcore
    ${TINYXML_LIBRARIES}
)

install(TARGETS webcore
        DESTINATION "${LIB_INSTALL_DIR}")
