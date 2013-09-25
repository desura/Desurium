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

if(WIN32)
  SetSharedRuntime(mcf_extract)
endif()

install_tool(mcf_extract)
