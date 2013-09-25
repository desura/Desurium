include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${CMAKE_CURRENT_SOURCE_DIR}/RES
)

configure_file(RES/service_exe.rc ${RC_DIR}/service_exe.rc @ONLY)
file(GLOB
  Sources code/*.cpp
  ${RC_DIR}/service_exe.rc
)

add_executable(desura_service ${Sources} ${COMMON_SOURCE})
target_link_libraries(desura_service
  mdump
  wintrust
)

if(WIN32)
  SetSharedRuntime(desura_service)
endif()

install_executable(desura_service)
