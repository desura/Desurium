include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(SQLITE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/ PARENT_SCOPE)

file(GLOB Headers ./include/*.h)
file(GLOB Sources ./code/*.c ./include/*.h)

add_definitions(-fPIC)
add_library(sqlite STATIC ${Sources})
target_link_libraries(sqlite
)

set(SQLITE_LIBRARIES sqlite PARENT_SCOPE)
