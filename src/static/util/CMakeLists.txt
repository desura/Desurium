include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CURL_INCLUDE_DIRS}
  ${SQLITE3X_INCLUDE_DIRS}
)

file(GLOB SourcesWEB code/UtilWeb.cpp)

add_library(util_web STATIC ${SourcesWEB})
target_link_libraries(util_web
    ${CURL_LIBRARIES}
)

file(GLOB Sources code/gcBuff.cpp
                  code/MD5Progressive.cpp
                  code/MD5Wrapper.cpp
                  code/MD5Wrapper.h
                  code/third_party
                  code/UtilBZip2.cpp
                  code/UtilFsPath.cpp
                  code/UtilMisc.cpp
                  code/UtilMisc_sha1.cpp
                  code/UtilOs.cpp
                  code/UtilString.cpp
                  code/third_party/GeneralHashFunctions.cpp
                  code/third_party/md5.cpp)

if(WIN32)
  file(GLOB PlattformSources code/gcDDE.cpp
                             code/gcDDEManager.cpp
                             code/gcDDEManager.h
                             code/UtilMisc_ico.cpp
                             code/UtilWindows.cpp
                             code/UtilWindows_dotnet.cpp
                             code/UtilWindows_firewall.cpp
                             code/UtilWindows_osver.cpp
                             code/UtilWindows_service.cpp)
else()
  file(GLOB PlattformSources code/UtilLinux.cpp)
endif()

add_library(util STATIC ${Sources} ${PlattformSources})

target_link_libraries(util
    ${Boost_LIBRARIES}
    ${BZIP2_LIBRARIES}
    ${SQLITE3X_LIBRARIES}
)
