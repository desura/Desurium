include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/RES
)

configure_file(RES/toolhelper.rc ${RC_DIR}/toolhelper.rc @ONLY)
file(GLOB
  Sources code/*.cpp
  ${RC_DIR}/toolhelper.rc
)
				
add_executable(toolhelper WIN32 ${Sources})
target_link_libraries(toolhelper
  mdump
  util_bootloader
)

install(TARGETS toolhelper
        DESTINATION "${BIN_INSTALL_DIR}")

SET_PROPERTY(TARGET toolhelper                PROPERTY FOLDER "Executable")