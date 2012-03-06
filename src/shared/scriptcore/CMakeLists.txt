include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
)

file(GLOB Sources code/*.cpp)

add_library(scriptcore SHARED ${Sources})
target_link_libraries(scriptcore
    gcJSBase
    scriptengine
    threads
    util
    util_fs
    ${TINYXML_LIBRARIES}
    ${V8_LIBRARIES}
)

install(TARGETS scriptcore
        DESTINATION "${LIB_INSTALL_DIR}")
