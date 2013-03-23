include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${Boost_INCLUDE_DIR}
  ${CEF_INCLUDE_DIRS}
  ${IPC_PIPE_INCLUDE_DIRS}
  ${SQLITE3X_INCLUDE_DIRS}
)

set(SERVICECORE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/code PARENT_SCOPE)

file(GLOB Sources code/ComplexLaunchProcess.cpp
                  code/InstallProcess.cpp
                  code/InstallScriptRunTime.cpp
                  code/Log.cpp
                  code/McfInit.cpp
                  code/ServiceCore.cpp
                  code/ServiceCoreMain.cpp
                  code/ServiceMainThread.cpp
                  code/UnInstallBranchProcess.cpp
                  code/UnInstallProcess.cpp
                  code/UnInstallRegKey.cpp
                  code/UpdateProcess.cpp
                  code/UpdateShortcuts.cpp
)

if(WIN32)
  file(GLOB PlattformSources
                  code/WindowsGameExplorer.cpp
  )
  set(PLATFORM_LIBRARIES msi Wintrust)
#  add_linker_flags(/NOD:LIBCMT)
else()
  file(GLOB PlattformSources)
  set(PLATFORM_LIBRARIES dl)
endif()

add_library(servicecore SHARED ${Sources} ${PlattformSources} ${SERVICE_PIPE_SPOURCES})
target_link_libraries(servicecore
  gcJSBase
  ipc_pipe
  threads
  umcf
  util
  util_fs
  ${BZIP2_LIBRARIES}
  ${SQLITE_LIBRARIES}
  ${SQLITE3X_LIBRARIES}
  ${TINYXML_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
  ${PLATFORM_LIBRARIES}
)

if(WIN32)
  SetSharedRuntime(servicecore)
endif()

install_library(servicecore)
