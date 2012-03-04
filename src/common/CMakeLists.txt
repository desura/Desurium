include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GLOBAL_INCLUDE_DIRS}
  ${CEF_INCLUDE_DIRS}
)
message("${CEF_INCLUDE_DIRS}")

set(COMMON_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

file(GLOB Headers ./*.h)
file(GLOB Sources ./*.h
                  gcJSBase.cpp)

add_definitions(-fPIC)
add_library(desuracommon STATIC ${Sources})
