###############################################################################
# copy branding
###############################################################################
set(BIN_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
file(MAKE_DIRECTORY ${BIN_INCLUDE_DIR})

file(COPY ${BRANDING} DESTINATION ${BIN_INCLUDE_DIR})
file(REMOVE_RECURSE ${BIN_INCLUDE_DIR}/branding)
file(RENAME ${BIN_INCLUDE_DIR}/${BRANDING} ${BIN_INCLUDE_DIR}/branding)

set(BR ${BIN_INCLUDE_DIR}/branding)

install(FILES ${BR}/data/desura_lin.png
        DESTINATION ${DATA_INSTALL_DIR}
        RENAME desura.png
)

install(FILES ${BR}/data/themes/default/images/app/about_logo.png
              ${BR}/data/themes/default/images/app/icon_download.png
              ${BR}/data/themes/default/images/app/signin_banner.png
        DESTINATION ${DATA_INSTALL_DIR}/themes/default/images/app/
)

###############################################################################
# do final steps for setting up the desurium build environment
###############################################################################

set(GLOBAL_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(COMMON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/common)

include_directories(
    ${GLOBAL_INCLUDE_DIRS}
    ${COMMON_INCLUDE_DIRS}
    ${TINYXML_INCLUDE_DIR}
    ${BIN_INCLUDE_DIR}
)

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
else()
  add_subdirectory(executable/bootloader_lin)
  add_subdirectory(executable/crashdlg_lin)
endif()

add_subdirectory(tools/process_data_folder)
add_subdirectory(tools/mfc_upload_cli)

###############################################################################
# we need some dependencies 
###############################################################################

#if(NOT wxWidgets_FOUND)
  add_dependencies(managers_window wxWidget-2-9)
  add_dependencies(wx_controls wxWidget-2-9)
  add_dependencies(Desurium wxWidget-2-9)
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
