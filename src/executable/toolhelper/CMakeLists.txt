include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/RES
)

file(GLOB Sources code/*.cpp)

add_linker_flags(/FORCE)
				
add_executable(toolhelper ${Sources})
target_link_libraries(toolhelper
  mdump
  util_bootloader
)

install(TARGETS toolhelper
        DESTINATION "${BIN_INSTALL_DIR}")
