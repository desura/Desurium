include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/RES
)

SetUseMFC()

configure_file(RES/repair.rc ${RC_DIR}/repair.rc @ONLY)
file(GLOB Sources
  code/*.cpp
  ${RC_DIR}/repair.rc
)

add_definitions(
  -Dgamecore_LOG
  -DDONTUSE_PCH
  -DDESURA_EXE
  -DDESURA_CLIENT
)
				
add_executable(utilhelper WIN32 ${Sources})
target_link_libraries(utilhelper
  mdump_s
  util_bootloader_s
)

install_executable(utilhelper)
SetStaticRuntime(utilhelper)
