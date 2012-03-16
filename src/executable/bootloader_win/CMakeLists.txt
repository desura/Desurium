include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${TINYXML_INCLUDE_DIR}
)

file(GLOB Sources code/*.cpp )

add_definitions(-D_WINDOWS
                -DGAMECORE_LOG
				-DDONT_INCLUDE_WINH
				-DDONTUSE_PCH
				-DDESURA_EXE
				-DDESURA_CLIENT
				-DNO_UTIL_FS)
add_linker_flags(/NOD:LIBCMT)
				
add_executable(Desurium ${Sources})
target_link_libraries(Desurium
    ipc_pipe
    mdump
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

install(TARGETS Desurium
        DESTINATION "${BIN_INSTALL_DIR}")
