include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${SERVICECORE_INCLUDE_DIRS}
)

set(IPC_PIPE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/code PARENT_SCOPE)

file(GLOB Sources code/IPCClass.cpp
                  code/IPCManager.cpp
                  code/IPCParameter.cpp
)

if(WIN32)
  file(GLOB PlattformSources code/IPCPipeBase.cpp
                             code/IPCPipeClient.cpp
                             code/IPCPipeServer.cpp
)
else()
  file(GLOB PlattformSources code/IPCPipeBase_Nix.cpp
                             code/IPCPipeClient_Nix.cpp
                             code/IPCPipeServer_Nix.cpp
)
endif()

add_library(ipc_pipe STATIC ${Sources} ${PlattformSources})

target_link_libraries(ipc_pipe
    threads
    util
    ${TINYXML_LIBRARIES}
)

SET_PROPERTY(TARGET ipc_pipe                PROPERTY FOLDER "Static")

if (WIN32)
	add_library(ipc_pipe_s STATIC ${Sources} ${PlattformSources})

	target_link_libraries(ipc_pipe_s
		threads_s
		util_s
		${TINYXML_LIBRARIES_S}
	)

	SET_PROPERTY(TARGET ipc_pipe_s                PROPERTY FOLDER "Static")
	SetStaticRuntime(ipc_pipe_s)
endif()