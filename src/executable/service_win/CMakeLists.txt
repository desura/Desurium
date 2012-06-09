include_directories(
)
#  ${CMAKE_CURRENT_SOURCE_DIR}/code
#  ${CEF_INCLUDE_DIRS}
#  ${IPC_PIPE_INCLUDE_DIRS}
#  ${BROWSER_PIPE_INCLUDE_DIRS}

file(GLOB Sources code/*.cpp )

add_definitions(
)
#  -D_CONSOLE
#  -DDONT_INCLUDE_AFXWIN
#  -DDONT_DEFINE_OS_VERSION
#add_linker_flags(/FORCE)
				
add_executable(service_win ${Sources} ${COMMON_SOURCE})
target_link_libraries(service_win
  mdump
  wintrust
)

install(TARGETS service_win
        DESTINATION "${BIN_INSTALL_DIR}")
