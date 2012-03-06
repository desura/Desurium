include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GLOBAL_INCLUDE_DIRS}
  ${COMMON_INCLUDE_DIRS}
  ${SQLITE3X_INCLUDE_DIRS}
)

file(GLOB Headers code/*.h code/third_party/.h)
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
                  code/UtilWeb.cpp
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

add_definitions(-fPIC)
add_library(util STATIC ${Sources} ${PlattformSources})
target_link_libraries(util
    sqlite3x
    ${TINYXML_LIBRARIES}
)
