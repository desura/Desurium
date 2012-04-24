set(BREAKPAD_EXCEPTION_HANDLER_INSTALL_DIR ${CMAKE_EXTERNAL_BINARY_DIR}/breakpad)
set(BREAKPAD_SVN http://google-breakpad.googlecode.com/svn/trunk -r 850)

if(WIN32)
  ExternalProject_Add(
    breakpad
    SVN_REPOSITORY ${BREAKPAD_SVN}
    UPDATE_COMMAND ""
    PATCH_COMMAND ${CMAKE_SCRIPT_PATH}/Patch.bat ${CMAKE_PATCH_DIR}/breakpad.patch
    CONFIGURE_COMMAND ${PYTHON_EXECUTABLE} ../breakpad/src/tools/gyp/gyp ../breakpad/src/client/windows/breakpad_client.gyp
    BUILD_COMMAND msbuild <SOURCE_DIR>/src/client/windows/handler/exception_handler.vcxproj /nologo /t:rebuild /m:2 /property:Configuration=Release
    INSTALL_COMMAND ""
  )
  ExternalProject_Add_Step(
    breakpad
    update_project_files
    DEPENDEES configure
    DEPENDERS build
    COMMAND devenv /upgrade <SOURCE_DIR>/src/client/windows/breakpad_client.sln
  )
else()
  ExternalProject_Add(
    breakpad
    SVN_REPOSITORY ${BREAKPAD_SVN}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND <SOURCE_DIR>/configure --enable-shared=no --enable-static=yes --prefix=${BREAKPAD_EXCEPTION_HANDLER_INSTALL_DIR}
  )
  ExternalProject_Add_Step(
    breakpad
    patch-bug-158
    COMMAND ${CMAKE_SCRIPT_PATH}/patch.sh ${CMAKE_PATCH_DIR}/bug-158.patch
    WORKING_DIRECTORY <SOURCE_DIR>
    DEPENDEES download
    DEPENDERS configure
  )
endif()

ExternalProject_Get_Property(
    breakpad
    source_dir
)
set(BREAKPAD_EXCEPTION_HANDLER_INCLUDE_DIR ${source_dir}/src)

if(WIN32)
  set(BREAKPAD_EXCEPTION_HANDLER_LIBRARY_DIR ${source_dir}/src/client/windows/handler/Release/lib)
  set(BREAKPAD_EXCEPTION_HANDLER_LIBRARIES "${BREAKPAD_EXCEPTION_HANDLER_LIBRARY_DIR}/exception_handler.lib")
else()
  set(BREAKPAD_EXCEPTION_HANDLER_LIBRARY_DIR ${BREAKPAD_EXCEPTION_HANDLER_INSTALL_DIR}/lib)
  set(BREAKPAD_EXCEPTION_HANDLER_LIBRARIES "${BREAKPAD_EXCEPTION_HANDLER_LIBRARY_DIR}/libbreakpad.a;${BREAKPAD_EXCEPTION_HANDLER_LIBRARY_DIR}/libbreakpad_client.a")
endif()
