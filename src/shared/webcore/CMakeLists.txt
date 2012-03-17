include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${Boost_INCLUDE_DIR}
  ${BZIP2_INCLUDE_DIR}
  ${SQLITE3X_INCLUDE_DIRS}
)

file(GLOB Sources code/*.cpp)

if(WIN32)
  set(WINDOWS_LIBRARIES crypt32 dnsapi rpcrt4 wininet winmm ws2_32)
  add_linker_flags(/NOD:LIBCMT)
endif()

add_library(webcore SHARED ${Sources})
target_link_libraries(webcore
    threads
    util
    util_fs
    util_web
	${BZIP2_LIBRARIES}
    ${CURL_LIBRARIES}
    ${TINYXML_LIBRARIES}
    ${SQLITE3X_LIBRARIES}
	${WINDOWS_LIBRARIES}
)

install(TARGETS webcore
        DESTINATION "${LIB_INSTALL_DIR}")
