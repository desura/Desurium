include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(SQLITE3X_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include/ PARENT_SCOPE)

file(GLOB Sqlite3xHeaders ./include/*.hpp)
file(GLOB Sqlite3xSources ./code/*.cpp ./include/*.hpp)

add_library(sqlite3x SHARED ${Sqlite3xSources})
