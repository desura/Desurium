include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${Boost_INCLUDE_DIR}
  ${SQLITE_INCLUDE_DIR}
)

set(SQLITE3X_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include PARENT_SCOPE)

file(GLOB Sqlite3xHeaders ${SQLITE3X_INCLUDE_DIRS}/sqlite3x.hpp)
file(GLOB Sqlite3xSources
  ./code/sqlite3x_command.cpp
  ./code/sqlite3x_connection.cpp
  ./code/sqlite3x_exception.cpp
  ./code/sqlite3x_reader.cpp
  ./code/sqlite3x_transaction.cpp
)

if(WIN32)
  add_definitions(-D_LIB -DSQLITE_THREADSAFE)
endif()

add_library(sqlite3x STATIC ${Sqlite3xSources} ${Sqlite3xHeaders})
target_link_libraries(sqlite3x ${SQLITE_LIBRARIES})
set(SQLITE3X_LIBRARIES sqlite3x PARENT_SCOPE)

set_property(TARGET sqlite3x PROPERTY FOLDER "ThirdParty")

if (WIN32)
	add_library(sqlite3x_s STATIC ${Sqlite3xSources} ${Sqlite3xHeaders})
	target_link_libraries(sqlite3x_s ${SQLITE_LIBRARIES_S})
	set(SQLITE3X_LIBRARIES_S sqlite3x_s PARENT_SCOPE)

	SET_PROPERTY(TARGET sqlite3x_s PROPERTY FOLDER "ThirdParty")
	SetStaticRuntime(sqlite3x_s)
endif()
