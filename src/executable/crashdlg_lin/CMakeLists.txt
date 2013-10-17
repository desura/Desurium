include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${GTK2_INCLUDE_DIRS}
)

file(GLOB Sources code/*.cpp )

add_executable(crashdlg ${Sources})
target_link_libraries(crashdlg
    rt
    util
    ${GTK2_LIBRARIES}
    ${SQLITE_LIBRARIES}
)

install_internal_tool(crashdlg)
