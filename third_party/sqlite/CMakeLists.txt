include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(SQLITE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/ PARENT_SCOPE)

file(GLOB Headers ./include/*.h)
file(GLOB Sources ./code/*.c ./include/*.h)

if(WIN32)
  add_definitions(/D_LIB /DSQLITE_THREADSAFE)
else()
  set(LINUX_LIBRARIES dl pthread)
endif()

add_library(sqlite STATIC ${Sources})
target_link_libraries(sqlite
  ${LINUX_LIBRARIES}
)

set(SQLITE_LIBRARIES sqlite PARENT_SCOPE)

SET_PROPERTY(TARGET sqlite                PROPERTY FOLDER "ThirdParty")