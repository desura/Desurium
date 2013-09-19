ExternalProject_Add(
	fetch_desura_extra_bin
	URL "${DESURA_EXTRA_BIN_URL}"
	URL_MD5 ${DESURA_EXTRA_BIN_MD5}
	UPDATE_COMMAND ""
	BUILD_IN_SOURCE 1
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
	INSTALL_COMMAND ""
)
	
ExternalProject_Get_Property(
	fetch_desura_extra_bin
	source_dir
)	

add_custom_command(TARGET fetch_desura_extra_bin POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${source_dir}\\." "${CMAKE_OUTPUT}\\bin\\.")
SET_PROPERTY(TARGET fetch_desura_extra_bin PROPERTY FOLDER "ThirdParty")
