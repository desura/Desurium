include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/src/common
  ${GLIB2_INCLUDE_DIRS}
  ${GTK2_INCLUDE_DIRS}
)

file(GLOB CommonSources
  src/common/debug.c
  src/common/npruntime.c
  src/common/npw-common.c
  src/common/npw-malloc.c
  src/common/npw-rpc.c
  src/common/rpc.c
  src/common/utils.c
)

file(GLOB WrapperSources
  src/wrapper/npw-wrapper.c
)

file(GLOB HostSources
  src/host/npw-viewer.c
)

if(64BIT)
  add_definitions(-DNIX64)
else()
  add_definitions(-DNIX32)
endif()
add_definitions(-std=c99)

add_library(desura_flashwrapper SHARED ${CommonSources} ${WrapperSources})
set_target_properties(desura_flashwrapper PROPERTIES COMPILE_DEFINITIONS BUILD_WRAPPER)
target_link_libraries(desura_flashwrapper
    ${SQLITE_LIBRARIES}
)

add_executable(desura_flashhost ${CommonSources} ${HostSources})
set_target_properties(desura_flashhost PROPERTIES COMPILE_DEFINITIONS BUILD_VIEWER)
target_link_libraries(desura_flashhost
  ${GTHREAD2_LIBRARIES}
  ${GTK2_LIBRARIES}
  ${X11_Xt_LIB}
)

install(TARGETS desura_flashwrapper
        DESTINATION "${LIB_INSTALL_DIR}")
install(TARGETS desura_flashhost
        DESTINATION "${LIB_INSTALL_DIR}")

