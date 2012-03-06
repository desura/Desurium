include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${COMMON_INCLUDE_DIRS}
  ${GLOBAL_INCLUDE_DIRS}
  ${TINYXML_INCLUDE_DIR}
)

file(GLOB DesuraSources code/*.cpp code/*.h )

add_definitions(-DDESURA_CLIENT)

add_executable(Desurium ${DesuraSources})
target_link_libraries(Desurium
    dl
    util
    mdump
    sqlite3x
    ${TINYXML_LIBRARIES}
    ${BZIP2_LIBRARIES}
    ${CURL_LIBRARIES}
    ${BREAKPAD_EXCEPTION_HANDLER_LIBRARIES}
    ${SQLITE_LIBRARIES}
    ${wxWidgets_LIBRARIES}
)

install(TARGETS Desurium
        DESTINATION "${BIN_INSTALL_DIR}")
