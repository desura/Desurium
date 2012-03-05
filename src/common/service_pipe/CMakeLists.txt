include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${COMMON_INCLUDE_DIRS}
  ${GLOBAL_INCLUDE_DIRS}
  ${IPC_PIPE_INCLUDE_DIRS}
  ${TINYXML_INCLUDE_DIRS}
  ${SERVICECORE_INCLUDE_DIRS}
)

file(GLOB Headers ./*.h)
file(GLOB Sources ./*.h
                  ./*.cpp)
set(SERVICE_PIPE_SPOURCES ${Sources} PARENT_SCOPE)

# add_definitions(-fPIC)
# add_library(service_pipe STATIC ${Sources})
