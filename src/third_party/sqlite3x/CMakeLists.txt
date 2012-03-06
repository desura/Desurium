include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${SQLITE_INCLUDE_DIRS}
)

set(SQLITE3X_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include/ PARENT_SCOPE)

file(GLOB Sqlite3xHeaders ./include/*.hpp)
file(GLOB Sqlite3xSources ./code/*.cpp ./include/*.hpp)

add_definitions(-fPIC)
add_library(sqlite3x STATIC ${Sqlite3xSources})
target_link_libraries(sqlite3x
    ${SQLITE_LIBRARIES}
)
set(SQLITE3X_LIBRARIES sqlite3x PARENT_SCOPE)
