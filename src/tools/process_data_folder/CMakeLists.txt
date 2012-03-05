include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${COMMON_INCLUDE_DIRS}
  ${GLOBAL_INCLUDE_DIRS}
)

file(GLOB Sources code/*.cpp code/*.h )

add_executable(process_data_folder ${Sources})
target_link_libraries(process_data_folder
    util
    util_fs
    ${SQLITE_LIBRARIES}
)
