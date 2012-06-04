include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${V8_INCLUDE_DIR}
)

file(GLOB Sources code/*.cpp)

if(WIN32)
  set(WINDOWS_LIBRARIES msi winmm Wintrust ws2_32)
#  add_linker_flags(/NOD:LIBCMT)
endif()

add_library(scriptcore SHARED ${Sources})
target_link_libraries(scriptcore
    gcJSBase
    scriptengine
    threads
    util
    util_fs
    ${TINYXML_LIBRARIES}
    ${V8_LIBRARIES}
	${WINDOWS_LIBRARIES}
)

install(TARGETS scriptcore
        DESTINATION "${LIB_INSTALL_DIR}")
