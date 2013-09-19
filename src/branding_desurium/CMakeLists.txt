ExternalProject_Add(
	update_branding
	DOWNLOAD_COMMAND ""
	UPDATE_COMMAND ""
	BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
)
	
add_custom_command(TARGET update_branding POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_SOURCE_DIR}\\data" "${CMAKE_OUTPUT}\\data")
SET_PROPERTY(TARGET update_branding PROPERTY FOLDER "Common")
