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

if(WIN32)
  SetSharedRuntime(mcf_upload_cli)
endif()

install_tool(mcf_upload_cli)
