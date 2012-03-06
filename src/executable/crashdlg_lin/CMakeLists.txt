include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${GTK2_INCLUDE_DIRS}
)

file(GLOB Sources code/*.cpp )

add_executable(crashdlg ${Sources})
target_link_libraries(crashdlg
    util
    ${GTK2_LIBRARIES}
    ${SQLITE_LIBRARIES}
)

install(TARGETS crashdlg
        DESTINATION "${BIN_INSTALL_DIR}")
