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

macro(RenameFile org dest)
	add_custom_command(TARGET update_branding POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_OUTPUT}\\data\\themes\\default\\images\\app\\${org}" "${CMAKE_OUTPUT}\\data\\themes\\default\\images\\app\\${dest}")
endmacro()

if(WIN32)
  RenameFile(menu_button_hover_win.png menu_button_hover.png)
  RenameFile(menu_button_nonactive_win.png menu_button_nonactive.png)
  RenameFile(menu_button_win.png menu_button.png)
else()
  RenameFile(menu_button_hover_lin.png menu_button_hover.png)
  RenameFile(menu_button_nonactive_lin.png menu_button_nonactive.png)
  RenameFile(menu_button_lin.png menu_button.png)
endif()

SET_PROPERTY(TARGET update_branding PROPERTY FOLDER "Common")
