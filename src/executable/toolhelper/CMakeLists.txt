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
  util_bootloader_s
)

install(TARGETS toolhelper
        DESTINATION "${BIN_INSTALL_DIR}")

CopyTargetFiles(toolhelper)
SET_PROPERTY(TARGET toolhelper                PROPERTY FOLDER "Executable")

SetStaticRuntime(toolhelper)