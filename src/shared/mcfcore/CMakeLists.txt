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
  if(NOT MINGW)
    set(PLATFORM_LIBRARIES dnsapi rpcrt4 wininet winmm ws2_32)
  else()
    set(PLATFORM_LIBRARIES ${WINDOWS_LIBRARIES} msi winhttp wintrust)
  endif()
#  add_linker_flags(/NOD:LIBCMT)
else()
  set(PLATFORM_LIBRARIES dl)
endif()

add_library(mcfcore SHARED ${Sources})
target_link_libraries(mcfcore
  umcf
  util
  util_fs
  util_web
  threads
  ${Boost_DATE_TIME_LIBRARY}
  ${BZIP2_LIBRARIES}
  ${CURL_LIBRARIES}
  ${SQLITE_LIBRARIES}
  ${SQLITE3X_LIBRARIES}
  ${TINYXML_LIBRARIES}
  ${PLATFORM_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)

install(TARGETS mcfcore
        DESTINATION "${LIB_INSTALL_DIR}")

CopyTargetFiles(mcfcore)
		
SET_PROPERTY(TARGET mcfcore                PROPERTY FOLDER "Shared")