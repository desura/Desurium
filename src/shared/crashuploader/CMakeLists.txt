include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${SQLITE3X_INCLUDE_DIRS}
)

file(GLOB Sources code/*.cpp)

if(WIN32)
#  add_linker_flags(/NODEFAULTLIB:LIBCMT)
endif()

add_library(crashuploader SHARED ${Sources})

target_link_libraries(crashuploader
    util
    util_fs
    util_web
    ${Boost_LIBRARIES}
    ${CURL_LIBRARIES}
    ${GTK2_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    ${TINYXML_LIBRARIES}
    ${windows_libs}
)

install(TARGETS crashuploader
        DESTINATION "${LIB_INSTALL_DIR}")
