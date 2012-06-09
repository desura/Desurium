include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${CEF_INCLUDE_DIRS}
  ${IPC_PIPE_INCLUDE_DIRS}
  ${BROWSER_PIPE_INCLUDE_DIRS}
)

file(GLOB Sources code/*.cpp )

add_definitions(
  -D_CONSOLE
  -DDONT_INCLUDE_AFXWIN
  -DDONT_DEFINE_OS_VERSION
)
add_linker_flags(/FORCE)
				
add_executable(desura_browserhost ${Sources} ${BROWSER_PIPE_SPOURCES})
target_link_libraries(desura_browserhost
  gcJSBase
  ipc_pipe
  mdump
  msi
  util
  util_fs
  wintrust
)

install(TARGETS desura_browserhost
        DESTINATION "${BIN_INSTALL_DIR}")
