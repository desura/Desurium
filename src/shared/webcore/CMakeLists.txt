include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${SQLITE3X_INCLUDE_DIRS}
)

file(GLOB Sources code/*.cpp)

add_library(webcore SHARED ${Sources})
target_link_libraries(webcore
    threads
    util
    util_fs
    util_web
    ${CURL_LIBRARIES}
    ${TINYXML_LIBRARIES}
    ${SQLITE3X_LIBRARIES}
)

install(TARGETS webcore
        DESTINATION "${LIB_INSTALL_DIR}")
