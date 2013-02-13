include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
)

file(GLOB Sources code/*.cpp )

if(UNIX)
  set(LINUX_LIBRARIES dl)
endif()

add_executable(mcf_extract ${Sources})
target_link_libraries(mcf_extract
  mcfcore
  ${LINUX_LIBRARIES}
)

install_executable(mcf_extract)
