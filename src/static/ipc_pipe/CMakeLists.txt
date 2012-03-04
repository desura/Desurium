include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GLOBAL_INCLUDE_DIRS}
  ${COMMON_INCLUDE_DIRS}
  ${TINYXML_INCLUDE_DIRS}
)

file(GLOB Headers code/*.h)
file(GLOB Sources code/IPCClass.cpp
                  code/IPCManager.cpp
                  code/IPCParamater.cpp
)

if(DEFINED WIN32)
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

add_library(ipc_pipe SHARED ${Sources} ${PlattformSources})
target_link_libraries(ipc_pipe
    tinyxml
)
