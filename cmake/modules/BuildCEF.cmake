if(BUILD_CEF OR BUILD_ONLY_CEF)
  set(DEPOT_TOOLS_INSTALL_DIR ${CMAKE_EXTERNAL_BINARY_DIR}/depot_tools)
  set(DEPOT_TOOLS_BIN_DIR ${DEPOT_TOOLS_INSTALL_DIR}/src/depot_tools)

  ProcessorCount(CPU_COUNT)

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

  configure_file(${CMAKE_PATCH_DIR}/cef_gyp.patch.inc ${CMAKE_BINARY_DIR}/gen/patches/cef_gyp.patch)
  ExternalProject_Add_Step(
    fetch_cef
    cef_gyp-patch
    COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_BINARY_DIR}/gen/patches/cef_gyp.patch
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

  if(NOT WIN32 OR MINGW)
    ExternalProject_Add_Step(
    cef
    copy_files
    COMMAND cp -r ${CEF_SOURCE_DIR} ./cef
    DEPENDERS download
    WORKING_DIRECTORY ${CHROMIUM_SOURCE_DIR}/src
    )

    ExternalProject_Add_Step(
    cef
    glib-2-32-patch
    COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_SOURCE_DIR}/cmake/patches/cef_glib_2_32_compile.patch
    DEPENDERS patch
    WORKING_DIRECTORY ${CHROMIUM_SOURCE_DIR}/src
    )

    ExternalProject_Add_Step(
    cef
    gcc-4-7-patch
    COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_SOURCE_DIR}/cmake/patches/cef_gcc47_compile_fix.patch
    DEPENDERS patch
    WORKING_DIRECTORY ${CHROMIUM_SOURCE_DIR}/src
    )

    ExternalProject_Add_Step(
    cef
    bison-2-6-patch
    COMMAND ${PATCH_SCRIPT_PATH} ${CMAKE_SOURCE_DIR}/cmake/patches/chromium-bison-2.6.patch
    DEPENDERS patch
    WORKING_DIRECTORY ${CHROMIUM_SOURCE_DIR}/src
    )

    ExternalProject_Add_Step(
    cef
    config_cef
    COMMAND ${CMAKE_SCRIPT_PATH}/depot_tools_wrapper.sh ${DEPOT_TOOLS_BIN_DIR} ./cef_create_projects.sh
    DEPENDEES download
    DEPENDERS configure
    WORKING_DIRECTORY ${CHROMIUM_SOURCE_DIR}/src/cef
    )

    ExternalProject_Add_Step(
    cef
    build_cef
    COMMAND ${CMAKE_SCRIPT_PATH}/depot_tools_wrapper.sh ${DEPOT_TOOLS_BIN_DIR} make cef_desura -j${CPU_COUNT} BUILDTYPE=Release
    DEPENDEES configure
    DEPENDERS build
    WORKING_DIRECTORY ${CHROMIUM_SOURCE_DIR}/src
    )
  else()
    ExternalProject_Add_Step(
    cef
    copy_files
    COMMAND ${CMAKE_SCRIPT_PATH}/xcopy.bat ${CEF_SOURCE_DIR} cef
    DEPENDERS download
    WORKING_DIRECTORY ${CHROMIUM_SOURCE_DIR}
    )
  endif()

  add_dependencies(cef depot_tools)
  add_dependencies(cef chromium)
  add_dependencies(cef fetch_cef)

  set(CEF_LIB_DIR ${CHROMIUM_SOURCE_DIR}/src/out/Release/lib.target)
  set(CEF_FFMPEG_LIB_DIR ${CHROMIUM_SOURCE_DIR}/src/out/Release)
  set(CEF_LIBRARIES "${CEF_LIB_DIR}/libcef_desura.so")
  set(CEF_FFMPEG_LIB "${CEF_FFMPEG_LIB_DIR}/libffmpegsumo.so")
  set(CEF_INCLUDE_DIRS "${CEF_SOURCE_DIR}")

  if(NOT WIN32)
    install(FILES ${CEF_LIBRARIES}
            DESTINATION ${LIB_INSTALL_DIR})
    install(FILES ${CEF_FFMPEG_LIB}
            DESTINATION ${LIB_INSTALL_DIR}/cefmedia)
  endif()
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
endif()

