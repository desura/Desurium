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

install(FILES ${IMAGES}
        DESTINATION ${DATA_INSTALL_DIR}/themes/default/images/app
)

install(FILES run.sh
        DESTINATION ${INSTALL_DIR}
)
