set(BUILD_SHARED_LIBS ON)

add_definitions("-DGTEST_CREATE_SHARED_LIBRARY=1")
add_definitions(-D_VARIADIC_MAX=10)

add_subdirectory(../gmock-1.6.0/gtest ${CMAKE_CURRENT_BINARY_DIR}/gtest_real)

#add_subdirectory(third_party/gtest-1.6.0)
#add_subdirectory(third_party/gmock-1.6.0/gtest)

#SET_PROPERTY(TARGET gmock PROPERTY FOLDER "ThirdParty")
#SET_PROPERTY(TARGET gmock_main PROPERTY FOLDER "ThirdParty")

SET_PROPERTY(TARGET gtest PROPERTY FOLDER "ThirdParty")
SET_PROPERTY(TARGET gtest_main PROPERTY FOLDER "ThirdParty")

if (WIN32)
	ExternalProject_Add(
		fetch_gtest
		DOWNLOAD_COMMAND ""
		UPDATE_COMMAND ""
		BUILD_IN_SOURCE 1
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
		DEPENDS gtest
	)

	SET_PROPERTY(TARGET fetch_gtest PROPERTY FOLDER "ThirdParty")
		
	get_target_property(target_loc gtest LOCATION)
	#CopyOutputFiles(fetch_gtest target_loc)
	
	add_custom_command(TARGET fetch_gtest POST_BUILD COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_OUTPUT}\\bin")
	add_custom_command(TARGET fetch_gtest POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different "${target_loc}" "${CMAKE_OUTPUT}\\bin\\.")	
endif()