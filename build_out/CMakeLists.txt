set(DATA ${CMAKE_CURRENT_SOURCE_DIR}/data)

###############################################################################
# /
###############################################################################

install(DIRECTORY 
                  ${DATA}/bindings
                  ${DATA}/language
                  ${DATA}/scripts
        DESTINATION ${DATA_INSTALL_DIR}
)

install(FILES ${DATA}/ca-bundle_lin.crt 
        DESTINATION ${DATA_INSTALL_DIR}
        RENAME ca-bundle.crt
)

install(FILES ${DATA}/cef_data_lin.pak
        DESTINATION ${DATA_INSTALL_DIR}
        RENAME cef_data.pak
)

###############################################################################
# /themes/default
###############################################################################

set(DT ${DATA}/themes/default)

install(DIRECTORY ${DT}/css
                  ${DT}/html
        DESTINATION ${DATA_INSTALL_DIR}/themes/default
)

install(FILES ${DT}/preview.jpg
              ${DT}/theme.xml
        DESTINATION ${DATA_INSTALL_DIR}/themes/default
)

###############################################################################
# /themes/default/js
###############################################################################

set(JS ${DT}/js)
file(GLOB JS_SCRIPTS ${JS}/*.js)

list(REMOVE_ITEM JS_SCRIPTS ${JS}/functions_lin.js
                            ${JS}/functions_win.js
                            ${JS}/settings_lin.js
                            ${JS}/settings_win.js)
install(FILES ${JS_SCRIPTS}
        DESTINATION ${DATA_INSTALL_DIR}/themes/default/js)

if(WIN32)
  install(FILES ${JS}/functions_win.js DESTINATION ${DATA_INSTALL_DIR}/themes/default/js RENAME functions.js)
  install(FILES ${JS}/settings_win.js DESTINATION ${DATA_INSTALL_DIR}/themes/default/js RENAME settings.js)
else()
  install(FILES ${JS}/functions_lin.js DESTINATION ${DATA_INSTALL_DIR}/themes/default/js RENAME functions.js)
  install(FILES ${JS}/settings_lin.js DESTINATION ${DATA_INSTALL_DIR}/themes/default/js RENAME settings.js)
endif()

###############################################################################
# /themes/default/images
###############################################################################

set(IMG ${DT}/images)

install(DIRECTORY ${IMG}/html
        DESTINATION ${DATA_INSTALL_DIR}/themes/default/images
)

###############################################################################
# /themes/default/images/app
###############################################################################

set(APP ${IMG}/app)
file(GLOB IMAGES ${APP}/*.png)

install(FILES ${IMAGES}
        DESTINATION ${DATA_INSTALL_DIR}/themes/default/images/app
)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/run.sh ${CMAKE_GEN_SRC_DIR}/build_out/run.sh @ONLY)
install(FILES ${CMAKE_GEN_SRC_DIR}/build_out/run.sh
        RENAME desura
        DESTINATION ${BIN_INSTALL_DIR}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ             GROUP_EXECUTE
                    WORLD_READ             WORLD_EXECUTE
)

install(FILES util.sh
        DESTINATION ${DATA_INSTALL_DIR}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ             GROUP_EXECUTE
                    WORLD_READ             WORLD_EXECUTE
)

install(FILES findlib.sh
        DESTINATION ${LIB_INSTALL_DIR}
        PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
                    GROUP_READ             GROUP_EXECUTE
                    WORLD_READ             WORLD_EXECUTE
)
