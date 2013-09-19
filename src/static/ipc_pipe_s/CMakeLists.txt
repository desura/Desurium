include_directories(
  ${IPC_PIPE_INCLUDE_DIRS}
  ${SERVICECORE_INCLUDE_DIRS}
)

file(GLOB Sources ${IPC_PIPE_INCLUDE_DIRS}/IPCClass.cpp
                  ${IPC_PIPE_INCLUDE_DIRS}/IPCManager.cpp
                  ${IPC_PIPE_INCLUDE_DIRS}/IPCParameter.cpp
)

file(GLOB PlattformSources ${IPC_PIPE_INCLUDE_DIRS}/IPCPipeBase.cpp
                             ${IPC_PIPE_INCLUDE_DIRS}/IPCPipeClient.cpp
                             ${IPC_PIPE_INCLUDE_DIRS}/IPCPipeServer.cpp
)

add_library(ipc_pipe_s STATIC ${Sources} ${PlattformSources})

target_link_libraries(ipc_pipe_s
	threads_s
	util_s
	${TINYXML_LIBRARIES_S}
)

SET_PROPERTY(TARGET ipc_pipe_s                PROPERTY FOLDER "Static")
SetStaticRuntime(ipc_pipe_s)
