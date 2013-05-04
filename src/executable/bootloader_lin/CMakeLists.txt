include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${TINYXML_INCLUDE_DIR}
)

file(GLOB Sources code/*.cpp )

if(WITH_BREAKPAD)
  set(MDUMP mdump)
endif()

add_executable(desura ${Sources})
target_link_libraries(desura
  dl
  ${MDUMP}
  rt
  sqlite3x
  util
  ${TINYXML_LIBRARIES}
  ${BZIP2_LIBRARIES}
  ${CURL_LIBRARIES}
  ${BREAKPAD_EXCEPTION_HANDLER_LIBRARIES}
  ${SQLITE_LIBRARIES}
  ${wxWidgets_LIBRARIES}
  ${LIBNOTIFY_LIBRARIES}
)

install_executable(desura)
        
