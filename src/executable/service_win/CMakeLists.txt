file(GLOB Sources code/*.cpp )
				
add_executable(service_win ${Sources} ${COMMON_SOURCE})
target_link_libraries(service_win
  mdump
  wintrust
)

install(TARGETS service_win
        DESTINATION "${BIN_INSTALL_DIR}")
