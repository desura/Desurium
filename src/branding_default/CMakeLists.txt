set(BR ${CMAKE_CURRENT_SOURCE_DIR})

install(DIRECTORY ${BR}/data/themes
        DESTINATION ${DATA_INSTALL_DIR}
)

install(FILES ${BR}/data/desura_lin.png
        DESTINATION ${DATA_INSTALL_DIR}
        RENAME desura.png
)
