###############################################################################
# copy branding
###############################################################################

set(BIN_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
set(RC_DIR ${CMAKE_BINARY_DIR}/rc_files)

file(MAKE_DIRECTORY ${BIN_INCLUDE_DIR})

file(COPY ${BRANDING} DESTINATION ${BIN_INCLUDE_DIR})
file(REMOVE_RECURSE ${BIN_INCLUDE_DIR}/branding)
file(RENAME ${BIN_INCLUDE_DIR}/${BRANDING} ${BIN_INCLUDE_DIR}/branding)

set(BRANDING_DIR ${BIN_INCLUDE_DIR}/branding)
set(APP ${BRANDING_DIR}/data/themes/default/images/app)

install_data_files("" ${BRANDING_DIR}/data/desura_lin.png)
install_data_files("" ${BRANDING_DIR}/data/desura_win.ico)
install_data_files(themes/default/images/html/default ${BRANDING_DIR}/data/themes/default/images/html/default/logo.png)
install_data_files(themes/default/images/app ${APP}/about_logo.png ${APP}/icon_download.png ${APP}/signin_banner.png)

if(WIN32)
  install_data_files(themes/default/images/app ${APP}/menu_button_hover_win.png)
  install_data_files(themes/default/images/app ${APP}/menu_button_nonactive_win.png)
  install_data_files(themes/default/images/app ${APP}/menu_button_win.png)
else()
  install_data_files(themes/default/images/app ${APP}/menu_button_hover_lin.png)
  install_data_files(themes/default/images/app ${APP}/menu_button_nonactive_lin.png)
  install_data_files(themes/default/images/app ${APP}/menu_button_lin.png)
endif()

###############################################################################
# do final steps for setting up the desura build environment
###############################################################################

if (WIN32)
	set(DESURA_EXE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/executable/bootloader_win/code)
endif()

set(GLOBAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(COMMON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/common)

if(ICC)
  # on ICC we have to include glib-2 because of some hacks in Common.h
  include_directories(
    ${GLOBAL_INCLUDE_DIRS}
    ${COMMON_INCLUDE_DIRS}
    ${TINYXML_INCLUDEDIR}
    ${BIN_INCLUDE_DIR}
    ${GTK2_INCLUDE_DIRS}
  )
else()
  include_directories(
    ${GLOBAL_INCLUDE_DIRS}
    ${COMMON_INCLUDE_DIRS}
    ${TINYXML_INCLUDEDIR}
    ${BIN_INCLUDE_DIR}
  )
endif()

###############################################################################
# build static libs
###############################################################################
add_subdirectory(common)
add_subdirectory(common/service_pipe)
if(WIN32)
  add_subdirectory(common/browser_pipe)
  add_subdirectory(common/tool_pipe)
endif()
add_subdirectory(static/managers)
add_subdirectory(static/mdump)
add_subdirectory(static/ipc_pipe)
add_subdirectory(static/scriptengine)
add_subdirectory(static/umcf)
add_subdirectory(static/util)

if(WIN32)
  add_subdirectory(static/util_bootloader)
  if(OFFICIAL_BUILD)
    add_subdirectory(static/util_bootloader_official)
  endif()
endif()
add_subdirectory(static/util_fs)
add_subdirectory(static/util_thread)
add_subdirectory(static/wx_controls)

###############################################################################
# build shared libs
###############################################################################

add_subdirectory(shared/crashuploader)
add_subdirectory(shared/mcfcore)
add_subdirectory(shared/servicecore)
add_subdirectory(shared/scriptcore)
add_subdirectory(shared/uicore)
if(WIN32)
  add_subdirectory(shared/utilcore)
endif()
add_subdirectory(shared/usercore)
add_subdirectory(shared/webcore)

###############################################################################
# build the client
###############################################################################

if(WIN32)
  add_subdirectory(executable/bootloader_win)
  add_subdirectory(executable/desura_browserhost)
  add_subdirectory(executable/service_win)
  add_subdirectory(executable/toolhelper)
  add_subdirectory(executable/utilhelper)
  add_subdirectory(executable/dumpgen_win)
else()
  add_subdirectory(executable/bootloader_lin)
  add_subdirectory(executable/crashdlg_lin)
  
  if(32BIT_SUPPORT)
    add_subdirectory(executable/bittest)
  endif()
endif()

option(BUILD_TOOLS "Build various Desura tools." ON)

if(BUILD_TOOLS)
  add_subdirectory(tools/mcf_util)
endif()

###############################################################################
# build the tests
###############################################################################

add_subdirectory(tests)

###############################################################################
# some pre building or installing actions
###############################################################################

# create a list of pairs first
set(DESURA_SYMLINK_RULES
    /usr/share/desura->/usr/bin;
    /usr/local/share/desura->/usr/local/bin;
    /usr/games/desura->/usr/games/bin;
    /opt/desura->/usr/bin;)

set(DESKTOP_EXE desura CACHE STRING "the value of Exe in the desktop file")
set(DESKTOP_ICON desura CACHE STRING "the value of Icon in the desktop file")

# configure our desktop file
configure_file(${CMAKE_SCRIPT_PATH}/desura.desktop ${CMAKE_BINARY_DIR}/desura.desktop @ONLY)
if(INSTALL_DESKTOP_FILE)
  # install it only if the option is turned on
  install(FILES ${CMAKE_BINARY_DIR}/desura.desktop
          DESTINATION ${DESKTOP_INSTALL_DIR}
          PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                      GROUP_READ             GROUP_EXECUTE
                      WORLD_READ             WORLD_EXECUTE)
endif()

# create symlink and install it
foreach(item ${DESURA_SYMLINK_RULES})
  pair(FIRST ${item} first)
  if(CMAKE_INSTALL_PREFIX STREQUAL "${first}" AND UNIX)
    pair(SECOND ${item} second)

    # to avoid the nedd of root rights while creating install packages we create
    # the symlink in CMAKE_BINARY_DIR and install it after creation
    install(CODE "execute_process(COMMAND ln -fs ${first}/run.sh ${CMAKE_BINARY_DIR}/desura)")
    install(FILES ${CMAKE_BINARY_DIR}/desura
            DESTINATION "${second}")
    message("II make install/package will create a symlink: ${second}/desura -> ${first}/run.sh")
    break()
  endif()
endforeach()


###############################################################################
# we need some dependencies 
###############################################################################

add_dependencies(gcJSBase fetch_cef)
add_dependencies(scriptengine fetch_cef)
add_dependencies(servicecore fetch_cef)
add_dependencies(uicore fetch_cef)
add_dependencies(usercore fetch_cef)
if(WIN32)
  add_dependencies(desura_browserhost fetch_cef)
  add_dependencies(utilcore fetch_cef)
endif()

#if(NOT wxWidgets_FOUND)
  add_dependencies(managers_window wxWidget-2-9)
  add_dependencies(wx_controls wxWidget-2-9)
  add_dependencies(uicore wxWidget-2-9)
  add_dependencies(desura wxWidget-2-9)
  if(WIN32)
    add_dependencies(utilcore wxWidget-2-9)
  endif()
#endif()

if(NOT Boost_FOUND)
  add_dependencies(managers boost)
  add_dependencies(mcfcore boost)
  add_dependencies(servicecore boost)
  add_dependencies(uicore boost)
  add_dependencies(usercore boost)
  add_dependencies(util boost)
  add_dependencies(util_fs boost)
  add_dependencies(threads boost)
  add_dependencies(webcore boost)
  add_dependencies(wx_controls boost)
endif()

if(NOT BREAKPAD_EXCEPTION_HANDLER_FOUND)
  add_dependencies(mdump breakpad)
endif()

if(NOT BZIP2_FOUND)
  add_dependencies(util bzip2)
endif()

if(NOT CURL_FOUND)
  add_dependencies(util curl)
  add_dependencies(util_web curl)
  add_dependencies(uicore curl)
endif()

if(NOT V8_FOUND)
  add_dependencies(scriptcore v8)
  add_dependencies(scriptengine v8)
endif()

if(NOT GTEST_FOUND AND WITH_GTEST)
  add_dependencies(uicore gtest)
endif()
