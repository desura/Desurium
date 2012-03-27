include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${TINYXML_INCLUDE_DIR}
)

file(GLOB Sources code/*.cpp )

add_executable(Desura ${Sources})
target_link_libraries(Desura
    dl
    mdump
    rt
    sqlite3x
    util
    ${TINYXML_LIBRARIES}
    ${BZIP2_LIBRARIES}
    ${CURL_LIBRARIES}
    ${BREAKPAD_EXCEPTION_HANDLER_LIBRARIES}
    ${SQLITE_LIBRARIES}
    ${wxWidgets_LIBRARIES}
)

install(TARGETS Desura
        DESTINATION "${BIN_INSTALL_DIR}")
