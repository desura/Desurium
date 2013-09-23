

set(UTIL_REAL_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../util)

include_directories(
  ${UTIL_REAL_SOURCE_DIR}
  ${UTIL_REAL_SOURCE_DIR}/code/third_party
  ${Boost_INCLUDE_DIR_S} 
  ${BZIP2_INCLUDE_DIR}
  ${SQLITE3X_INCLUDE_DIRS_S}
)

file(GLOB SourcesWEB ${UTIL_REAL_SOURCE_DIR}/code/UtilWeb.cpp)


# configure source file to hardcode paths
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../util/code/UtilOs_cmake.cpp.inc ${CMAKE_GEN_SRC_DIR}/util/UtilOs_cmake.cpp)

file(GLOB Sources ${UTIL_REAL_SOURCE_DIR}/code/gcBuff.cpp
                  ${UTIL_REAL_SOURCE_DIR}/code/MD5Progressive.cpp
                  ${UTIL_REAL_SOURCE_DIR}/code/MD5Wrapper.cpp
                  ${UTIL_REAL_SOURCE_DIR}/code/MD5Wrapper.h
                  ${UTIL_REAL_SOURCE_DIR}/code/third_party
                  ${UTIL_REAL_SOURCE_DIR}/code/UtilBZip2.cpp
                  ${UTIL_REAL_SOURCE_DIR}/code/UtilFsPath.cpp
                  ${UTIL_REAL_SOURCE_DIR}/code/UtilMisc.cpp
                  ${UTIL_REAL_SOURCE_DIR}/code/UtilMisc_sha1.cpp
                  ${UTIL_REAL_SOURCE_DIR}/code/UtilOs.cpp
                  ${UTIL_REAL_SOURCE_DIR}/code/UtilString.cpp
                  ${UTIL_REAL_SOURCE_DIR}/code/third_party/GeneralHashFunctions.cpp
                  ${UTIL_REAL_SOURCE_DIR}/code/third_party/md5.cpp
                  ${CMAKE_GEN_SRC_DIR}/util/UtilOs_cmake.cpp)

  file(GLOB PlattformSources ${UTIL_REAL_SOURCE_DIR}/code/gcDDE.cpp
                             ${UTIL_REAL_SOURCE_DIR}/code/gcDDEManager.cpp
                             ${UTIL_REAL_SOURCE_DIR}/code/gcDDEManager.h
                             ${UTIL_REAL_SOURCE_DIR}/code/UtilMisc_ico.cpp
                             ${UTIL_REAL_SOURCE_DIR}/code/UtilWindows.cpp
                             ${UTIL_REAL_SOURCE_DIR}/code/UtilWindows_dotnet.cpp
                             ${UTIL_REAL_SOURCE_DIR}/code/UtilWindows_firewall.cpp
                             ${UTIL_REAL_SOURCE_DIR}/code/UtilWindows_osver.cpp
                             ${UTIL_REAL_SOURCE_DIR}/code/UtilWindows_service.cpp
							 ${UTIL_REAL_SOURCE_DIR}/code/third_party/WinXPSP2FireWall.cpp)
  set(PLATFORM_LIBRARIES msi wintrust)
use_unicode_here()

add_library(util_s STATIC ${Sources} ${PlattformSources})

target_link_libraries(util_s
  ${BZIP2_LIBRARIES_S}
  ${SQLITE3X_LIBRARIES_S}
  ${Boost_FILESYSTEM_LIBRARY_S}
  ${Boost_SYSTEM_LIBRARY_S}
  ${PLATFORM_LIBRARIES}
)

SET_PROPERTY(TARGET util_s                PROPERTY FOLDER "Static")
SetStaticRuntime(util_s)

if(WIN32 AND NOT MINGW)
  set(WINDOWS_LIBRARIES msi Winhttp wintrust)
  use_unicode_here()
endif()

add_library(util_web_s STATIC ${SourcesWEB})
target_link_libraries(util_web_s
	${WINDOWS_LIBRARIES}
	${CURL_LIBRARIES_S}
)

SET_PROPERTY(TARGET util_web_s                PROPERTY FOLDER "Static")
SetStaticRuntime(util_web_s)	