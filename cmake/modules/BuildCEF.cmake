if(UNIX)
  set(MY_BUILD_TOOL $(MAKE))
else()
  set(MY_BUILD_TOOL ${CMAKE_BUILD_TOOL})
endif()

if(BUILD_CEF OR BUILD_ONLY_CEF)
  if(WIN32)
    if(NOT SUBVERSION_FOUND)
      find_package(Subversion REQUIRED)
    endif()
    find_package(DirectX REQUIRED)
    if(NOT DirectX_D3DX9_FOUND)
      message(FATAL_ERROR "DirectX9 SDK not found")
    else()
      message("-- DirectX9 found: ${DirectX_D3DX9_LIBRARY} ${DirectX_D3DX9_INCLUDE_DIR}")
    endif()
  endif()

  set(DEPOT_TOOLS_INSTALL_DIR ${CMAKE_EXTERNAL_BINARY_DIR}/depot_tools)
  set(DEPOT_TOOLS_BIN_DIR ${DEPOT_TOOLS_INSTALL_DIR}/src/depot_tools)

  ExternalProject_Add(
    depot_tools
    URL ${DEPOT_TOOLS_URL}
    URL_MD5 ${DEPOT_TOOLS_MD5}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    INSTALL_COMMAND ""
    PREFIX ${DEPOT_TOOLS_INSTALL_DIR}
  )

  if(PYTHON_VERSION_MAJOR EQUAL 3)
    ExternalProject_Add_Step(
    depot_tools
    fix_python_scripts
    COMMAND ${CMAKE_SCRIPT_PATH}/fix_python_depot_tools.sh
    DEPENDEES build
    WORKING_DIRECTORY ${DEPOT_TOOLS_BIN_DIR}
    )
  endif()
  
  if(NOT WIN32 OR MINGW)
    set(chromium_INSTALL_COMMAND "${CMAKE_SCRIPT_PATH}/fix_chromium_path.sh")
  else()
    set(chromium_INSTALL_COMMAND "${CMAKE_SCRIPT_PATH}/fix_chromium_path.bat")
  endif()
  
  ExternalProject_Add(
    chromium
    URL ${CHROMIUM_URL}
    URL_MD5 ${CHROMIUM_MD5}
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND ""
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
  )
  
  ExternalProject_Get_Property(
    chromium
    source_dir
  )
  set(CHROMIUM_SOURCE_DIR ${source_dir})
  
  ExternalProject_Add_Step(
    chromium
    chromium_move
    COMMAND "${chromium_INSTALL_COMMAND}"
    DEPENDERS install
    DEPENDEES download
    WORKING_DIRECTORY ${CHROMIUM_SOURCE_DIR}/..
  )
  
  ExternalProject_Add(
    fetch_cef
    URL ${CEF_URL}
    URL_MD5 ${CEF_MD5}
    UPDATE_COMMAND ""
    PATCH_COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_SOURCE_DIR}/cmake/patches/cef.patch
    CONFIGURE_COMMAND ""
    BUILD_COMMAND "" 
    INSTALL_COMMAND ""
  )

  ExternalProject_Get_Property(
    fetch_cef
    source_dir
  )
  set(FETCH_CEF_SOURCE_DIR ${source_dir})

  ExternalProject_Add_Step(
    fetch_cef
    cef_gyp-patch
    COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_PATCH_DIR}/cef_gyp.patch
    DEPENDEES patch
    WORKING_DIRECTORY ${FETCH_CEF_SOURCE_DIR}
  )
  
  if(WIN32)
    configure_file(${CMAKE_PATCH_DIR}/cef_gyp_gclient_hook_win.patch.inc ${CMAKE_BINARY_DIR}/gen/patches/cef_gyp_gclient_hook.patch)
  else()
    configure_file(${CMAKE_PATCH_DIR}/cef_gyp_gclient_hook_lin.patch.inc ${CMAKE_BINARY_DIR}/gen/patches/cef_gyp_gclient_hook.patch)
  endif()
  ExternalProject_Add_Step(
    fetch_cef
    cef_gyp_gclient_hook-patch
    COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_BINARY_DIR}/gen/patches/cef_gyp_gclient_hook.patch
    DEPENDEES patch
    WORKING_DIRECTORY ${FETCH_CEF_SOURCE_DIR}
  )

  ExternalProject_Add(
    cef
    DOWNLOAD_COMMAND ""
    UPDATE_COMMAND ""
    CONFIGURE_COMMAND ""
    BUILD_COMMAND "" 
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND ""
  )

  ExternalProject_Get_Property(
    fetch_cef
    source_dir
  )
  set(CEF_SOURCE_DIR ${source_dir})
  
  # set some environment dependent variables
  if(WIN32 AND NOT MINGW)
    set(WORKING_DIR ${CHROMIUM_SOURCE_DIR}/src)
    set(CEF_COPY_CMD ${CMAKE_SCRIPT_PATH}/xcopy.bat ${CEF_SOURCE_DIR} cef)
    set(CEF_CONFIG_CMD ${CMAKE_SCRIPT_PATH}/depot_tools_wrapper.bat ${DEPOT_TOOLS_BIN_DIR} ${PYTHON_EXECUTABLE} tools/gclient_hook.py)
    if(DEBUG_CEF)
      set(CEF_BUILD_CMD msbuild ${WORKING_DIR}/cef/cef.sln /target:cef_desura /consoleloggerparameters:verbosity=minimal /m /p:Platform=Win32 /p:Configuration=Debug)
      set(CEF_LIB_DIR ${WORKING_DIR}/cef/Debug/)
    else()
      set(CEF_BUILD_CMD msbuild ${WORKING_DIR}/cef/cef.sln /target:cef_desura /consoleloggerparameters:verbosity=minimal /m /p:Platform=Win32 /p:Configuration=Release)
      set(CEF_LIB_DIR ${WORKING_DIR}/cef/Release/)
    endif()
    set(CEF_LIBRARIES "${CEF_LIB_DIR}/cef_desura.dll" "${CEF_LIB_DIR}/icudt.dll")
    set(CEF_FFMPEG_LIB "${CEF_LIB_DIR}/avcodec-53.dll" "${CEF_LIB_DIR}/avformat-53.dll" "${CEF_LIB_DIR}/avutil-51.dll")
    set(CEF_MEDIA_DIR "${LIB_INSTALL_DIR}")
  else()
    set(WORKING_DIR ${CHROMIUM_SOURCE_DIR}/src)
    set(CEF_COPY_CMD cp -r ${CEF_SOURCE_DIR} ./cef)
    set(CEF_CONFIG_CMD ${CMAKE_SCRIPT_PATH}/depot_tools_wrapper.sh ${DEPOT_TOOLS_BIN_DIR} ./cef_create_projects.sh)
    set(CEF_BUILD_CMD ${CMAKE_SCRIPT_PATH}/depot_tools_wrapper.sh ${DEPOT_TOOLS_BIN_DIR} ${MY_BUILD_TOOL} cef_desura V=1 $ENV{MAKEOPTS} CC.host=${CMAKE_C_COMPILER} CXX.host=${CMAKE_CXX_COMPILER} LINK.host=${CMAKE_CXX_COMPILER} AR.host=${CMAKE_AR} BUILDTYPE=Release)

    set(CEF_LIB_DIR ${CHROMIUM_SOURCE_DIR}/src/out/Release/lib.target)
    set(CEF_FFMPEG_LIB_DIR ${CHROMIUM_SOURCE_DIR}/src/out/Release)
    set(CEF_LIBRARIES "${CEF_LIB_DIR}/libcef_desura.so")
    set(CEF_FFMPEG_LIB "${CEF_FFMPEG_LIB_DIR}/libffmpegsumo.so")
    set(CEF_MEDIA_DIR "${LIB_INSTALL_DIR}/cefmedia")
    
    set(ENV{CFLAGS.host} "$ENV{CFLAGS}")
    set(ENV{CXXFLAGS.host} "$ENV{CXXFLAGS}")
    set(ENV{LDFLAGS.host} "$ENV{LDFLAGS}")
  endif()
  
  set(CEF_INCLUDE_DIRS "${CEF_SOURCE_DIR}")
  
  ExternalProject_Add_Step(
    cef
    copy_files
    COMMAND ${CEF_COPY_CMD}
    DEPENDERS download
    WORKING_DIRECTORY ${WORKING_DIR}
  )
  
  ExternalProject_Add_Step(
    cef
    nss-3-15-patch
    COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_SOURCE_DIR}/cmake/patches/chromium-nss-3.15.patch
    DEPENDERS patch
    WORKING_DIRECTORY ${CHROMIUM_SOURCE_DIR}/src
    )

    ExternalProject_Add_Step(
    cef
    config_cef
    COMMAND ${CEF_CONFIG_CMD}
    DEPENDEES download
    DEPENDERS configure
    WORKING_DIRECTORY ${WORKING_DIR}/cef
  )
  
  ExternalProject_Add_Step(
    cef
    build_cef
    COMMAND ${CEF_BUILD_CMD}
    DEPENDEES configure
    DEPENDERS build
    WORKING_DIRECTORY ${WORKING_DIR}
  )
  
  if(WIN32 AND NOT MINGW)
    # we need to download some external libs
    set(DEFAULT_SVN_URL https://src.chromium.org/chrome/trunk)
    macro(download_cef_dep name svn_url path)
      ExternalProject_Add_Step(
        cef
        ${name}-svn-fetch
        COMMAND ${Subversion_SVN_EXECUTABLE} co ${svn_url} ${WORKING_DIR}/${path} --force
        DEPENDERS config_cef
        WORKING_DIRECTORY ${WORKING_DIR}
      )
    endmacro()
    download_cef_dep(nss ${DEFAULT_SVN_URL}/deps/third_party/nss@94921 third_party/nss)
    download_cef_dep(yasm ${DEFAULT_SVN_URL}/deps/third_party/yasm/binaries@74228 third_party/yasm/binaries)
    download_cef_dep(rlz https://rlz.googlecode.com/svn/trunk@47 rlz)
    download_cef_dep(nacl http://src.chromium.org/native_client/trunk/src/native_client/tests@6668 native_client/tests)
    download_cef_dep(cygwin ${DEFAULT_SVN_URL}/deps/third_party/cygwin@66844 third_party/cygwin)
    download_cef_dep(ffmpeg_bin ${DEFAULT_SVN_URL}/deps/third_party/ffmpeg/binaries/win@99115 third_party/ffmpeg/binaries/chromium/win/ia32)
  else()
    # some patches for Linux
    ExternalProject_Add_Step(
      cef
      glib-2-32-patch
      COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_PATCH_DIR}/cef_glib_2_32_compile.patch
      DEPENDERS patch
      WORKING_DIRECTORY ${WORKING_DIR}
    )

    ExternalProject_Add_Step(
      cef
      gcc-4-7-patch
      COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_PATCH_DIR}/cef_gcc47_compile_fix.patch
      DEPENDERS patch
      WORKING_DIRECTORY ${WORKING_DIR}
    )

    ExternalProject_Add_Step(
      cef
      bison-2-6-patch
      COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_PATCH_DIR}/chromium-bison-2.6.patch
      DEPENDERS patch
      WORKING_DIRECTORY ${WORKING_DIR}
    )
  endif()
  
  add_dependencies(cef depot_tools)
  add_dependencies(cef chromium)
  add_dependencies(cef fetch_cef)

  install(FILES ${CEF_LIBRARIES}
          DESTINATION ${LIB_INSTALL_DIR})
  install(FILES ${CEF_FFMPEG_LIB}
          DESTINATION ${CEF_MEDIA_DIR})
		  
  SET_PROPERTY(TARGET cef                PROPERTY FOLDER "ThirdParty")
else(BUILD_CEF)
  ExternalProject_Add(
    fetch_cef
    URL ${CEF_URL}
    URL_MD5 ${CEF_MD5}
    UPDATE_COMMAND ""
    PATCH_COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_SOURCE_DIR}/cmake/patches/cef.patch
    CONFIGURE_COMMAND ""
    BUILD_COMMAND "" 
    INSTALL_COMMAND ""
  )
  
  ExternalProject_Get_Property(
    fetch_cef
    source_dir
  )
  set(CEF_SOURCE_DIR ${source_dir})
  set(CEF_INCLUDE_DIRS "${CEF_SOURCE_DIR}")
  
  SET_PROPERTY(TARGET fetch_cef PROPERTY FOLDER "ThirdParty")
  
  if (WIN32)
	ExternalProject_Add(
	  fetch_cef_bin
	  URL "${CEF_BIN_URL}"
	  URL_MD5 ${CEF_BIN_MD5}
	  UPDATE_COMMAND ""
	  BUILD_IN_SOURCE 1
	  CONFIGURE_COMMAND ""
	  BUILD_COMMAND ""
	  INSTALL_COMMAND ""
	)
	
    ExternalProject_Get_Property(
	  fetch_cef_bin
  	  source_dir
    )	
	
	add_custom_command(TARGET fetch_cef_bin POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${source_dir}\\." "${CMAKE_OUTPUT}\\bin\\.")
	SET_PROPERTY(TARGET fetch_cef_bin PROPERTY FOLDER "ThirdParty")
  endif()
endif()

