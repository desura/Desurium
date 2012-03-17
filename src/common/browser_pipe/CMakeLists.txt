include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${IPC_PIPE_INCLUDE_DIRS}
)

file(GLOB Sources *.cpp)

set(BROWSER_PIPE_SPOURCES ${Sources} PARENT_SCOPE)
