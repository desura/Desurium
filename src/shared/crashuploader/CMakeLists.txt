include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${SQLITE3X_INCLUDE_DIRS}
)

file(GLOB Sources code/*.cpp)

if(WIN32)
  if(MINGW)
    set(PLATFORM_LIBRARIES winhttp wintrust msi)
  endif()
#  add_linker_flags(/NODEFAULTLIB:LIBCMT)
endif()

add_library(crashuploader SHARED ${Sources})

target_link_libraries(crashuploader
    util
    util_fs
    util_web
    ${CURL_LIBRARIES}
    ${GTK2_LIBRARIES}
    ${TINYXML_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${PLATFORM_LIBRARIES}
)

install(TARGETS crashuploader
        DESTINATION "${LIB_INSTALL_DIR}")

SET_PROPERTY(TARGET crashuploader                PROPERTY FOLDER "Shared")