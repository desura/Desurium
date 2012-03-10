include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
)

file(GLOB Sources code/*.cpp )

add_executable(mfc_upload_cli ${Sources})
target_link_libraries(mfc_upload_cli
    rt
    managers
    managers_window
    threads
    usercore
    util
    util_fs
    ${Boost_LIBRARIES}
    ${CARES_LIBRARIES}
    ${OPENSSL_LIBRARIES}
)

install(TARGETS mfc_upload_cli
        DESTINATION "${BIN_INSTALL_DIR}")
