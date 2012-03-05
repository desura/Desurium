set(DATA ${CMAKE_CURRENT_SOURCE_DIR}/data)

install(DIRECTORY ${DATA}/themes
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
