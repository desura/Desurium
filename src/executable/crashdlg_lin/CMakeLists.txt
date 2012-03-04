include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${COMMON_INCLUDE_DIRS}
  ${GLOBAL_INCLUDE_DIRS}
  ${GTK2_INCLUDE_DIRS}
  ${TINYXML_INCLUDE_DIRS}
)

file(GLOB Sources code/*.cpp code/*.h )

add_executable(crashdlg ${Sources})
target_link_libraries(crashdlg
    util
    ${GTK2_LIBRARIES}
    ${SQLITE_LIBRARIES}
)
