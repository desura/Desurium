include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
)

file(GLOB Sources code/*.cpp )

if(WIN32)

else()
  set(LINUX_LIBRARIES rt)
endif()

add_executable(process_data_folder ${Sources})
target_link_libraries(process_data_folder
    util
    util_fs
    ${Boost_LIBRARIES}
    ${SQLITE_LIBRARIES}
    ${TINYXML_LIBRARIES}
	${LINUX_LIBRARIES}
)

install(TARGETS process_data_folder
        DESTINATION "${BIN_INSTALL_DIR}")
