include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${Boost_INCLUDE_DIR}
  ${BZIP2_INCLUDE_DIR}
  ${SQLITE3X_INCLUDE_DIRS}
)

file(GLOB Sources code/*.cpp)

if(WIN32)
  set(WINDOWS_LIBRARIES )
  if(NOT MINGW)
    set(WINDOWS_LIBRARIES ${WINDOWS_LIBRARIES} crypt32 dnsapi rpcrt4 wininet winmm ws2_32)
  else()
    set(WINDOWS_LIBRARIES ${WINDOWS_LIBRARIES} msi wintrust winhttp)
  endif()
endif()

add_library(webcore SHARED ${Sources} ${TINYXML2_CPP})
target_link_libraries(webcore
    threads
    util
    util_fs
    util_web
	${BZIP2_LIBRARIES}
    ${CURL_LIBRARIES}
    ${TINYXML_LIBRARIES}
    ${SQLITE3X_LIBRARIES}
	${Boost_LIBRARIES}
	${WINDOWS_LIBRARIES}
	${CMAKE_THREAD_LIBS_INIT}
)

install(TARGETS webcore
        DESTINATION "${LIB_INSTALL_DIR}")
