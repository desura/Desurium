include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
)

file(GLOB Sources code/*.cpp )

add_executable(mfc_upload_cli ${Sources})
target_link_libraries(mfc_upload_cli
    managers
    managers_window
    threads
    usercore
    util
    util_fs
    ${CURL_LIBRARIES}
    ${Boost_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)

install_executable(mfc_upload_cli)
