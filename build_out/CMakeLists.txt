set(DATA ${CMAKE_CURRENT_SOURCE_DIR}/data)

###############################################################################
# /
###############################################################################

install_data_directories("" ${DATA}/bindings)
install_data_directories("" ${DATA}/language)
install_data_directories("" ${DATA}/scripts)
install_data_files("" "${DATA}/ca-bundle_lin.crt")
install_data_files("" "${DATA}/cef_data_lin.pak")

###############################################################################
# /themes/default
###############################################################################

set(DT ${DATA}/themes/default)

install_data_directories("themes/default" "${DT}/css")
install_data_directories("themes/default" "${DT}/html")
install_data_files("themes/default" ${DT}/preview.jpg ${DT}/theme.xml)

###############################################################################
# /themes/default/js
###############################################################################

set(JS ${DT}/js)
file(GLOB JS_SCRIPTS ${JS}/*.js)

list(REMOVE_ITEM JS_SCRIPTS ${JS}/functions_lin.js
                            ${JS}/functions_win.js
                            ${JS}/settings_lin.js
                            ${JS}/settings_win.js)
install_data_files("themes/default/js" ${JS_SCRIPTS})

if(WIN32)
  install_data_files("themes/default/js" ${JS}/functions_win.js)
  install_data_files("themes/default/js" ${JS}/settings_win.js)
else()
  install_data_files("themes/default/js" ${JS}/functions_lin.js)
  install_data_files("themes/default/js" ${JS}/settings_lin.js)
endif()

###############################################################################
# /themes/default/images
###############################################################################

set(IMG ${DT}/images)

install_data_directories("themes/default/images" ${IMG}/html)

###############################################################################
# /themes/default/images/app
###############################################################################

set(APP ${IMG}/app)
file(GLOB IMAGES ${APP}/*.png ${APP}/*.gif)

install_data_files("themes/default/images/app" ${IMAGES})

if(NOT WIN32)
  install_internal_tool_script(util.sh)
  install_internal_tool_script(findlib.sh)
endif()
