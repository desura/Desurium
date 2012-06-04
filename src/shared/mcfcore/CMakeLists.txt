include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${Boost_INCLUDE_DIR}
  ${BZIP2_INCLUDE_DIR}
  ${COURGETTE_INCLUDE_DIRS}
)

file(GLOB Sources code/*.cpp
                  code/mcf/*.cpp
                  code/thread/*.cpp
)

add_definitions(-DNEED_STRDUP -DNEED_MEMCCPY)

if(WIN32)
  set(WINDOWS_LIBRARIES dnsapi rpcrt4 wininet winmm ws2_32)
#  add_linker_flags(/NOD:LIBCMT)
endif()

add_library(mcfcore SHARED ${Sources})
target_link_libraries(mcfcore
    umcf
    util
    util_fs
    util_web
    threads
    ${Boost_LIBRARIES}
	${BZIP2_LIBRARIES}
    ${CURL_LIBRARIES}
    ${SQLITE_LIBRARIES}
    ${SQLITE3X_LIBRARIES}
    ${TINYXML_LIBRARIES}
	${WINDOWS_LIBRARIES}
)

install(TARGETS mcfcore
        DESTINATION "${LIB_INSTALL_DIR}")
