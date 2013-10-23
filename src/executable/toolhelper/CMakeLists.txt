include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/RES
  ${UTIL_BOOTLOADER_INCLUDE_DIRS}
)

configure_file(RES/toolhelper.rc ${RC_DIR}/toolhelper.rc @ONLY)
file(GLOB
  Sources code/*.cpp
  ${RC_DIR}/toolhelper.rc
)
				
add_executable(toolhelper WIN32 ${Sources})
target_link_libraries(toolhelper
  mdump_s
  util_s
  util_bootloader_s
  Comctl32
)

install_executable(toolhelper)
SetStaticRuntime(toolhelper)
