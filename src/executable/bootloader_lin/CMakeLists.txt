include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${TINYXML_INCLUDE_DIR}
)

file(GLOB Sources code/*.cpp )

add_executable(Desurium ${Sources})
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
