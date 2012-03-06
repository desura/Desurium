include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${COURGETTE_INCLUDE_DIRS}
)

file(GLOB Sources code/*.cpp
                  code/mcf/*.cpp
                  code/thread/*.cpp
)

add_definitions(-DNEED_STRDUP -DNEED_MEMCCPY)

add_library(mcfcore SHARED ${Sources})
target_link_libraries(mcfcore
    umcf
    util
    util_fs
    threads
    ${Boost_LIBRARIES}
    ${CURL_LIBRARIES}
    ${SQLITE_LIBRARIES}
    ${SQLITE3X_LIBRARIES}
    ${TINYXML_LIBRARIES}
)

install(TARGETS mcfcore
        DESTINATION "${LIB_INSTALL_DIR}")
