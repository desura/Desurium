include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${CMAKE_CURRENT_SOURCE_DIR}/RES
  ${COMMON_INCLUDE_DIRS}/service_pipe
  ${IPC_PIPE_INCLUDE_DIRS}
  ${TINYXML_INCLUDEDIR}
  ${UTIL_BOOTLOADER_OFFICIAL_INCLUDE_DIRS}
  ${UTIL_BOOTLOADER_INCLUDE_DIRS}
)

configure_file(RES/desura_exe.rc ${RC_DIR}/desura_exe.rc @ONLY)
file(GLOB Sources code/*.cpp ${RC_DIR}/desura_exe.rc)

set(Sources "${Sources};${AdditionalSources}")

add_definitions(-D_WINDOWS
  -DGAMECORE_LOG
  -DDONTUSE_PCH
  -DDESURA_EXE
  -DDESURA_CLIENT
  -DNO_UTIL_FS
)

add_executable(desura WIN32 ${Sources} ${SERVICE_PIPE_IPCUPDATEMAIN})

if (OFFICIAL_BUILD)
	set(OFFICIAL_DEP "util_bootloader_official_s")
	add_definitions(-DDESURA_OFFICAL_BUILD)
endif()


if (WIN32)
	target_link_libraries(desura
	  ipc_pipe_s
	  threads_s
	  util_s
	  util_bootloader_s
	  comctl32
	  mdump_s
	  ${OFFICIAL_DEP}
	)
	SetStaticRuntime(desura)
else()
	target_link_libraries(desura
	  ipc_pipe
	  threads
	  util
	  util_bootloader
	  comctl32
	  mdump
	)
endif()

install(TARGETS desura
        DESTINATION "${BIN_INSTALL_DIR}")

CopyTargetFiles(desura)
SET_PROPERTY(TARGET desura                PROPERTY FOLDER "Executable")