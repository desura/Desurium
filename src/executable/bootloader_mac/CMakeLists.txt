include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
)

file(GLOB Sources code/*.mm code/*.h)

add_executable(desura ${Sources})

target_link_libraries(desura
  util
  ${FOUNDATION_LIBRARY}
)

install_executable(desura)
