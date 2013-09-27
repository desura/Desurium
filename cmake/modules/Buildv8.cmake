if(64BIT)
  set(EXTRA_OPTS "-Dv8_target_arch=x64")
else()
  set(EXTRA_OPTS "-Dv8_target_arch=ia32")
endif()

if(UNIX)
  set(ENV{CFLAGS.host} "$ENV{CFLAGS}")
  set(ENV{CXXFLAGS.host} "$ENV{CXXFLAGS}")
  set(ENV{LDFLAGS.host} "$ENV{LDFLAGS}")
  set(V8_CONFIGURE_CMD ${CMAKE_SCRIPT_PATH}/depot_tools_wrapper.sh <SOURCE_DIR>/build gyp_v8 -Dcomponent=shared_library -Dwerror= --generator-output=out -f make ${EXTRA_OPTS})
  set(V8_BUILD_CMD $(MAKE) $ENV{MAKEOPTS} V=1 -C out BUILDTYPE=Release CC.host=${CMAKE_C_COMPILER} CXX.host=${CMAKE_CXX_COMPILER} LINK.host=${CMAKE_CXX_COMPILER} AR.host=${CMAKE_AR})
else()
  if(NOT SUBVERSION_FOUND)
    find_package(Subversion REQUIRED)
  endif()
  set(V8_CONFIGURE_CMD ${PYTHON_EXECUTABLE} <SOURCE_DIR>/build/gyp_v8 -Dcomponent=shared_library -f msvs -Dtarget_arch=ia32)
  if(DEBUG_V8)
    set(V8_BUILD_CMD msbuild <SOURCE_DIR>/tools/gyp/v8.sln /m /p:Platform=Win32 /p:Configuration=Debug)
  else()
    set(V8_BUILD_CMD msbuild <SOURCE_DIR>/tools/gyp/v8.sln /m /p:Platform=Win32 /p:Configuration=Release)
  endif()
endif()

ExternalProject_Add(
  v8
  URL ${V8_URL}
  URL_MD5 ${V8_MD5}
  CONFIGURE_COMMAND ${V8_CONFIGURE_CMD}
  BUILD_COMMAND ${V8_BUILD_CMD}
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND ""
)
if(WIN32)

ExternalProject_Add_Step(
  v8
  cygwin-svn-fetch
  COMMAND ${Subversion_SVN_EXECUTABLE} co http://src.chromium.org/svn/trunk/deps/third_party/cygwin@66844 third_party/cygwin
  DEPENDEES download
  DEPENDERS configure
  DEPENDEES download
  WORKING_DIRECTORY <SOURCE_DIR>
)
endif()

ExternalProject_Get_Property(
  v8
  source_dir
)

set(V8_INSTALL_DIR ${source_dir})
set(V8_INCLUDE_DIR ${V8_INSTALL_DIR}/include)

if(WIN32)
  if(DEBUG_V8)
    set(V8_LIBRARIES "${V8_INSTALL_DIR}/build/Debug/lib/v8.lib")
    install(FILES "${V8_INSTALL_DIR}/build/Debug/v8.dll"
            DESTINATION ${LIB_INSTALL_DIR})
  else()
    set(V8_LIBRARIES "${V8_INSTALL_DIR}/build/Release/lib/v8.lib")
    install(FILES "${V8_INSTALL_DIR}/build/Release/v8.dll"
            DESTINATION ${LIB_INSTALL_DIR})
  endif()
else()
  if(DEBUG_V8)
    set(V8_LIBRARIES "${V8_INSTALL_DIR}/out/out/Debug/lib.target/libv8.so")
  else()
    set(V8_LIBRARIES "${V8_INSTALL_DIR}/out/out/Release/lib.target/libv8.so")
  endif()
  install(FILES ${V8_LIBRARIES}
          DESTINATION ${LIB_INSTALL_DIR})
endif()


SET_PROPERTY(TARGET v8                PROPERTY FOLDER "ThirdParty")
