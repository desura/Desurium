set(BR ${CMAKE_CURRENT_SOURCE_DIR})

install(FILES ${BR}/data/desura_lin.png
        DESTINATION ${DATA_INSTALL_DIR}
        RENAME desura.png
)

install(FILES ${BR}/data/themes/default/images/app/signin_banner.png
              ${BR}/data/themes/default/images/app/about_logo.png
        DESTINATION ${DATA_INSTALL_DIR}/themes/default/images/app/
)
