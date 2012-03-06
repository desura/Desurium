set(DATA ${CMAKE_CURRENT_SOURCE_DIR}/data)

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

set(DT ${DATA}/themes/default)

install(DIRECTORY ${DT}/css
                  ${DT}/html
                  ${DT}/js
        DESTINATION ${DATA_INSTALL_DIR}/themes/default
)

install(FILES ${DT}/preview.jpg
              ${DT}/theme.xml
        DESTINATION ${DATA_INSTALL_DIR}/themes/default
)

set(IMG ${DT}/images)

install(DIRECTORY ${IMG}/html
        DESTINATION ${DATA_INSTALL_DIR}/themes/default/images
)

set(APP ${IMG}/app)

file(GLOB IMAGES ${APP}/*.png)
file(GLOB IMAGES_REMOVE ${APP}/*win.png
                        ${APP}/*lin.png)

list(REMOVE_ITEM IMAGES ${IMAGES_REMOVE})

install(FILES ${IMAGES}
        DESTINATION ${DATA_INSTALL_DIR}/themes/default/images/app
)

if(WIN32)
  install(FILES ${APP}/menu_button_hover_win.png RENAME menu_button_hover.png DESTINATION ${DATA_INSTALL_DIR}/themes/default/images/app)
  install(FILES ${APP}/menu_button_nonactive_win.png RENAME menu_button_nonactive.png DESTINATION ${DATA_INSTALL_DIR}/themes/default/images/app)
  install(FILES ${APP}/menu_button_win.png RENAME menu_button.png DESTINATION ${DATA_INSTALL_DIR}/themes/default/images/app)
else()
  install(FILES ${APP}/menu_button_hover_lin.png RENAME menu_button_hover.png DESTINATION ${DATA_INSTALL_DIR}/themes/default/images/app)
  install(FILES ${APP}/menu_button_nonactive_lin.png RENAME menu_button_nonactive.png DESTINATION ${DATA_INSTALL_DIR}/themes/default/images/app)
  install(FILES ${APP}/menu_button_lin.png RENAME menu_button.png DESTINATION ${DATA_INSTALL_DIR}/themes/default/images/app)
endif()
