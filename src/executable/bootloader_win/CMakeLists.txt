include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${CMAKE_CURRENT_SOURCE_DIR}/RES
  ${COMMON_INCLUDE_DIRS}/service_pipe
  ${IPC_PIPE_INCLUDE_DIRS}
  ${TINYXML_INCLUDE_DIR}
)

file(GLOB Sources code/*.cpp )

add_definitions(-D_WINDOWS
                -DGAMECORE_LOG
				-DDONT_INCLUDE_WINH
				-DDONTUSE_PCH
				-DDESURA_EXE
				-DDESURA_CLIENT
				-DNO_UTIL_FS
				-D_AFXDLL)
add_linker_flags(/NOD:LIBCMT /FORCE)
				
add_executable(Desura ${Sources} ${SERVICE_PIPE_IPCUPDATEMAIN})
target_link_libraries(Desura
    ipc_pipe
    mdump
	msi
    sqlite3x
	threads
    util
	util_bootloader
    ${TINYXML_LIBRARIES}
    ${BZIP2_LIBRARIES}
    ${CURL_LIBRARIES}
    ${BREAKPAD_EXCEPTION_HANDLER_LIBRARIES}
    ${SQLITE_LIBRARIES}
    ${wxWidgets_LIBRARIES}
	comctl32 comsuppw odbc32 Psapi rpcrt4 winmm wintrust wsock32
)

install(TARGETS Desura
        DESTINATION "${BIN_INSTALL_DIR}")
