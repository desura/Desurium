include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
)

file(GLOB Sources code/*.cpp )

add_executable(mcf_extract ${Sources})
target_link_libraries(mcf_extract
    mcfcore
    ${OPENSSL_LIBRARIES}
)

install(TARGETS mcf_extract
        DESTINATION "${BIN_INSTALL_DIR}")
