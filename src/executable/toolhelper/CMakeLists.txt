include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/RES
)

configure_file(RES/toolhelper.rc ${RC_DIR}/toolhelper.rc @ONLY)
file(GLOB
  Sources code/*.cpp
  ${RC_DIR}/toolhelper.rc
)

add_linker_flags(/FORCE)
				
add_executable(toolhelper ${Sources})
target_link_libraries(toolhelper
  mdump
  util_bootloader
)

install(TARGETS toolhelper
        DESTINATION "${BIN_INSTALL_DIR}")
