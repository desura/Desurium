include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/RES
)

SetUseMFC()

configure_file(RES/toolhelper.rc ${RC_DIR}/toolhelper.rc @ONLY)
file(GLOB
  Sources code/*.cpp
  ${RC_DIR}/toolhelper.rc
)
				
add_executable(toolhelper WIN32 ${Sources})
target_link_libraries(toolhelper
  mdump_s
  util_bootloader_s
)

install_executable(toolhelper)
SetStaticRuntime(toolhelper)
