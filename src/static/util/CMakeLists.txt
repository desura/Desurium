include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/code/third_party
  ${Boost_INCLUDE_DIR}
  ${BZIP2_INCLUDE_DIR}
  ${CURL_INCLUDE_DIRS}
  ${CURL_INCLUDE_DIRS_S}
  ${SQLITE3X_INCLUDE_DIRS}
)

file(GLOB SourcesWEB code/UtilWeb.cpp)

if(WIN32 AND NOT MINGW)
  set(WINDOWS_LIBRARIES msi Winhttp wintrust)
  use_unicode_here()
endif()

add_library(util_web STATIC ${SourcesWEB})
target_link_libraries(util_web
	${WINDOWS_LIBRARIES}
    ${CURL_LIBRARIES}
)

# configure source file to hardcode paths
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/code/UtilOs_cmake.cpp.inc ${CMAKE_GEN_SRC_DIR}/util/UtilOs_cmake.cpp)

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
                  code/third_party/md5.cpp
                  ${CMAKE_GEN_SRC_DIR}/util/UtilOs_cmake.cpp)

if(WIN32)
  file(GLOB PlattformSources code/gcDDE.cpp
                             code/gcDDEManager.cpp
                             code/gcDDEManager.h
                             code/UtilMisc_ico.cpp
                             code/UtilWindows.cpp
                             code/UtilWindows_dotnet.cpp
                             code/UtilWindows_firewall.cpp
                             code/UtilWindows_osver.cpp
                             code/UtilWindows_service.cpp
							 code/third_party/WinXPSP2FireWall.cpp)
  set(PLATFORM_LIBRARIES msi wintrust)
  use_unicode_here()
else()
  file(GLOB PlattformSources code/UtilLinux.cpp)
endif()

if(UNIX)
  if(NOT DIR_TYPE)
    set(DIR_TYPE "SINGLE" CACHE STRING "The directories for Desura's settings. Options: XDG SINGLE PORTABLE")
  endif()
  
  if(${DIR_TYPE} MATCHES "XDG")
    add_definitions(-DUSE_XDG_DIRS)
  elseif(${DIR_TYPE} MATCHES "SINGLE")
    add_definitions(-DUSE_SINGLE_HOME_DIR)
  elseif(${DIR_TYPE} MATCHES "PORTABLE")
    add_definitions(-DUSE_PORTABLE_DIR)
  else()
    message(FATAL_ERROR "Unknown DIR_TYPE!")
  endif()
endif()

add_library(util STATIC ${Sources} ${PlattformSources})
target_link_libraries(util
  ${BZIP2_LIBRARIES}
  ${SQLITE3X_LIBRARIES}
  ${Boost_FILESYSTEM_LIBRARY}
  ${Boost_DATE_TIME_LIBRARY}
  ${Boost_SYSTEM_LIBRARY}
  ${PLATFORM_LIBRARIES}
)

if(WIN32)
  SetSharedRuntime(util)
  SetSharedRuntime(util_web)
endif()

SET_PROPERTY(TARGET util                PROPERTY FOLDER "Static")
SET_PROPERTY(TARGET util_web                PROPERTY FOLDER "Static")

if (WIN32)
	add_library(util_s STATIC ${Sources} ${PlattformSources})

	target_link_libraries(util_s
	  ${BZIP2_LIBRARIES_S}
	  ${SQLITE3X_LIBRARIES_S}
	  ${Boost_FILESYSTEM_LIBRARY_S}
      ${Boost_SYSTEM_LIBRARY_S}
	  ${Boost_DATE_TIME_LIBRARY_S}
	  ${PLATFORM_LIBRARIES}
	)

	SET_PROPERTY(TARGET util_s                PROPERTY FOLDER "Static")
	SetStaticRuntime(util_s)
	
	
	add_library(util_web_s STATIC ${SourcesWEB})
	target_link_libraries(util_web_s
		${WINDOWS_LIBRARIES}
		${CURL_LIBRARIES_S}
	)
	
	SET_PROPERTY(TARGET util_web_s                PROPERTY FOLDER "Static")
	SetStaticRuntime(util_web_s)
endif()
