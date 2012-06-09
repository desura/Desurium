include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/RES
)

configure_file(RES/repair.rc ${RC_DIR}/repair.rc @ONLY)
file(GLOB Sources
  code/*.cpp
  ${RC_DIR}/repair.rc
)

add_definitions(
  -Dgamecore_LOG
  -DDONT_INCLUDE_WINH
  -DDONTUSE_PCH
  -DDESURA_EXE
  -DDESURA_CLIENT
)
add_linker_flags(/FORCE)
				
add_executable(utilhelper ${Sources})
target_link_libraries(utilhelper
  mdump
  util_bootloader
)

install(TARGETS utilhelper
        DESTINATION "${BIN_INSTALL_DIR}")
