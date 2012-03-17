include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${IPC_PIPE_INCLUDE_DIRS}
  ${SERVICECORE_INCLUDE_DIRS}
)

file(GLOB Sources *.cpp)
file(GLOB Sources_ipc_update_main IPCUpdateApp.cpp)

set(SERVICE_PIPE_SPOURCES ${Sources} PARENT_SCOPE)
set(SERVICE_PIPE_IPCUPDATEMAIN ${Sources_ipc_update_main} PARENT_SCOPE)
