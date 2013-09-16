include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(SQLITE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/ PARENT_SCOPE)

file(GLOB Headers
  ${SQLITE_INCLUDE_DIR}/sqlite3.h
  ${SQLITE_INCLUDE_DIR}/sqlite3ext.h)

file(GLOB Sources
  ./code/sqlite3.c
  ./code/sqlite3ext.c)

if(WIN32)
  add_definitions(-D_LIB -DSQLITE_THREADSAFE)
else()
  set(LINUX_LIBRARIES dl ${CMAKE_THREAD_LIBS_INIT})
endif()

add_library(sqlite STATIC ${Sources} ${Headers})
target_link_libraries(sqlite
  ${LINUX_LIBRARIES}
)

set(SQLITE_LIBRARIES sqlite PARENT_SCOPE)

SET_PROPERTY(TARGET sqlite                PROPERTY FOLDER "ThirdParty")
