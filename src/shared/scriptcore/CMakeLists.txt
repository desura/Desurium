include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${GLOBAL_INCLUDE_DIRS}
  ${COMMON_INCLUDE_DIRS}
  ${SQLITE3X_INCLUDE_DIRS}
  ${V8_INCLUDE_DIR}
)

file(GLOB Headers code/*.h)
file(GLOB Sources code/*.cpp
)

if(WIN32)
  file(GLOB PlattformSources)
else()
  file(GLOB PlattformSources)
endif()

add_library(scriptcore SHARED ${Sources} ${PlattformSources})
target_link_libraries(scriptcore
    ${TINYXML_LIBRARIES}
)

install(TARGETS scriptcore
        DESTINATION "${LIB_INSTALL_DIR}")
