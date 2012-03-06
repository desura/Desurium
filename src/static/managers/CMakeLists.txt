include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${GLOBAL_INCLUDE_DIRS}
  ${COMMON_INCLUDE_DIRS}
  ${SQLITE3X_INCLUDE_DIRS}
  ${wxWidgets_INCLUDE_DIRS}
)

file(GLOB Sources code/*.cpp code/*.h)

add_definitions(-fPIC)
add_library(managers STATIC ${Sources})
target_link_libraries(managers
    ${TINYXML_LIBRARIES}
)
