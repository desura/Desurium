include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${CMAKE_CURRENT_SOURCE_DIR}/RES
  ${COMMON_INCLUDE_DIRS}/service_pipe
  ${IPC_PIPE_INCLUDE_DIRS}
  ${TINYXML_INCLUDE_DIR}
)

configure_file(RES/desura_exe.rc ${RC_DIR}/desura_exe.rc @ONLY)
file(GLOB Sources code/AppUpdateInstall.cpp
                  code/Log.cpp
                  code/UpdateFunctions.cpp
                  ${RC_DIR}/desura_exe.rc)

if(MFC_FOUND)
  file(GLOB AdditionalSources code/DesuraMain.cpp)
  # set "Use of MFC" to "Use MFC in a Shared DLL" in VS
  add_definitions(-D_AFXDLL)
  set(CMAKE_MFC_FLAG 2)
else()
  file(GLOB AdditionalSources code/DesuraMainMingw.cpp)
endif()
set(Sources "${Sources};${AdditionalSources}")

add_definitions(-D_WINDOWS
                -DGAMECORE_LOG
				-DDONT_INCLUDE_WINH
				-DDONTUSE_PCH
				-DDESURA_EXE
				-DDESURA_CLIENT
				-DNO_UTIL_FS)
				
add_executable(desura ${Sources} ${SERVICE_PIPE_IPCUPDATEMAIN})
target_link_libraries(desura
    ipc_pipe
	threads
    util
    util_bootloader
	comctl32
)

install(TARGETS desura
        DESTINATION "${BIN_INSTALL_DIR}")
