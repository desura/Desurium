set(BREAKPAD_EXCEPTION_HANDLER_INSTALL_DIR ${CMAKE_EXTERNAL_BINARY_DIR}/breakpad)

if(WIN32 AND NOT MINGW)
  if(DEBUG)
    set(CONFIGURATION_TYPE Debug)
  else()
    set(CONFIGURATION_TYPE Release)
  endif()
  
  ExternalProject_Add(
    breakpad
    URL ${BREAKPAD_URL}
    URL_MD5 ${BREAKPAD_MD5}
    UPDATE_COMMAND ""
    PATCH_COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_PATCH_DIR}/breakpad.patch
    CONFIGURE_COMMAND ${PYTHON_EXECUTABLE} ../breakpad/src/tools/gyp/gyp ../breakpad/src/client/windows/breakpad_client.gyp
    BUILD_COMMAND msbuild <SOURCE_DIR>/src/client/windows/handler/exception_handler.vcxproj /nologo /t:rebuild /m:2 /property:Configuration=${CONFIGURATION_TYPE}
    INSTALL_COMMAND ""
  )
  ExternalProject_Add_Step(
    breakpad
    update_project_files
    DEPENDEES configure
    DEPENDERS build
    COMMAND vcupgrade <SOURCE_DIR>/src/client/windows/handler/exception_handler.vcproj
  )
  # Breakpad builds with /MT by default but we need /MD. This patch makes it build with /MD
  ExternalProject_Add_Step(
    breakpad
    patch_project_files
    DEPENDEES update_project_files
    DEPENDERS build
    WORKING_DIRECTORY <SOURCE_DIR>
    COMMAND cmake -DVCXPROJ_PATH=<SOURCE_DIR>/src/client/windows/handler/exception_handler.vcxproj -P ${CMAKE_SCRIPT_PATH}/breakpad_VS_patch.cmake
  )
  
  
  
  ExternalProject_Add(
    breakpad_s
    URL ${BREAKPAD_URL}
    URL_MD5 ${BREAKPAD_MD5}
    UPDATE_COMMAND ""
    PATCH_COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_PATCH_DIR}/breakpad.patch
    CONFIGURE_COMMAND ${PYTHON_EXECUTABLE} ../breakpad_s/src/tools/gyp/gyp ../breakpad_s/src/client/windows/breakpad_client.gyp
    BUILD_COMMAND msbuild <SOURCE_DIR>/src/client/windows/handler/exception_handler.vcxproj /nologo /t:rebuild /m:2 /property:Configuration=${CONFIGURATION_TYPE}
    INSTALL_COMMAND ""
  )
  ExternalProject_Add_Step(
    breakpad_s
    update_project_files
    DEPENDEES configure
    DEPENDERS build
    COMMAND vcupgrade <SOURCE_DIR>/src/client/windows/handler/exception_handler.vcproj
  )
  
	ExternalProject_Get_Property(
	  breakpad_s
	  source_dir
	)
	set(BREAKPAD_EXCEPTION_HANDLER_INCLUDE_DIR_S ${source_dir}/src)  
	set(BREAKPAD_EXCEPTION_HANDLER_LIBRARY_DIR_S ${source_dir}/src/client/windows/handler/${CONFIGURATION_TYPE}/lib)
	set(BREAKPAD_EXCEPTION_HANDLER_LIBRARIES_S "${BREAKPAD_EXCEPTION_HANDLER_LIBRARY_DIR_S}/exception_handler.lib")	
	SET_PROPERTY(TARGET breakpad_s PROPERTY FOLDER "ThirdParty")
else()
  ExternalProject_Add(
    breakpad
    URL ${BREAKPAD_URL}
    URL_MD5 ${BREAKPAD_MD5}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --enable-shared=no --enable-static=yes --prefix=${BREAKPAD_EXCEPTION_HANDLER_INSTALL_DIR}
  )
  ExternalProject_Add_Step(
    breakpad
    patch-bug-158
    COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_PATCH_DIR}/bug-158.patch
    WORKING_DIRECTORY <SOURCE_DIR>
    DEPENDEES download
    DEPENDERS configure
  )
  if(MINGW)
    ExternalProject_Add_Step(
      breakpad
      patch-mingw
      COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_PATCH_DIR}/breakpad-mingw.patch
      WORKING_DIRECTORY <SOURCE_DIR>
      DEPENDEES download
      DEPENDERS configure
    )
    ExternalProject_Add_Step(
      breakpad
      patch-windows
      COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_PATCH_DIR}/breakpad.patch
      WORKING_DIRECTORY <SOURCE_DIR>
      DEPENDEES download
      DEPENDERS configure
    )
  endif()
endif()

ExternalProject_Get_Property(
  breakpad
  source_dir
)
set(BREAKPAD_EXCEPTION_HANDLER_INCLUDE_DIR ${source_dir}/src)

if(WIN32 AND NOT MINGW)
  set(BREAKPAD_EXCEPTION_HANDLER_LIBRARY_DIR ${source_dir}/src/client/windows/handler/${CONFIGURATION_TYPE}/lib)
  set(BREAKPAD_EXCEPTION_HANDLER_LIBRARIES "${BREAKPAD_EXCEPTION_HANDLER_LIBRARY_DIR}/exception_handler.lib")
else()
  set(BREAKPAD_EXCEPTION_HANDLER_LIBRARY_DIR ${BREAKPAD_EXCEPTION_HANDLER_INSTALL_DIR}/lib)
  set(BREAKPAD_EXCEPTION_HANDLER_LIBRARIES "${BREAKPAD_EXCEPTION_HANDLER_LIBRARY_DIR}/libbreakpad.a")
  if(NOT MINGW)
    set(BREAKPAD_EXCEPTION_HANDLER_LIBRARIES "${BREAKPAD_EXCEPTION_HANDLER_LIBRARIES};${BREAKPAD_EXCEPTION_HANDLER_LIBRARY_DIR}/libbreakpad_client.a")
  endif()
endif()

SET_PROPERTY(TARGET breakpad                PROPERTY FOLDER "ThirdParty")
