include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${CMAKE_CURRENT_SOURCE_DIR}/RES
)

configure_file(RES/service_exe.rc ${RC_DIR}/service_exe.rc @ONLY)
file(GLOB
  Sources code/*.cpp
  ${RC_DIR}/service_exe.rc
)

add_executable(service_win ${Sources} ${COMMON_SOURCE})
target_link_libraries(service_win
  mdump
  wintrust
)

install(TARGETS service_win
        DESTINATION "${BIN_INSTALL_DIR}")
