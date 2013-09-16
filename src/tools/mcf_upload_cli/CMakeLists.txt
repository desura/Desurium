include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
)

file(GLOB Sources code/*.cpp )

add_executable(mcf_upload_cli ${Sources})
target_link_libraries(mcf_upload_cli
    managers
    managers_window
    threads
    usercore
    util
    util_fs
    ${CURL_LIBRARIES}
)

install_executable(mcf_upload_cli)

SET_PROPERTY(TARGET mcf_upload_cli                PROPERTY FOLDER "Tools")