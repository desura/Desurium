include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${TINYXML_INCLUDE_DIR}
)

file(GLOB Sources code/*.cpp )

if(UNIX)
  set(LINUX_LIBRARIES notify)
endif()

add_executable(desura ${Sources})
target_link_libraries(desura
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
  ${LINUX_LIBRARIES}
)

install(TARGETS desura
        DESTINATION "${BIN_INSTALL_DIR}")
