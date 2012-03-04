include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${COMMON_INCLUDE_DIRS}
  ${GLOBAL_INCLUDE_DIRS}
  ${TINYXML_INCLUDE_DIRS}
)

file(GLOB DesuraSources code/*.cpp code/*.h )

add_executable(Desura ${DesuraSources})
target_link_libraries(Desura
    dl
    util
    mdump
    sqlite3x
    ${BZIP2_LIBRARIES}
    ${CURL_LIBRARIES}
    ${BREAKPAD_EXCEPTION_HANDLER_LIBRARIES}
    ${SQLITE_LIBRARIES})

