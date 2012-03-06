include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GLOBAL_INCLUDE_DIRS}
  ${COMMON_INCLUDE_DIRS}
  ${CEF_INCLUDE_DIRS}
  ${V8_INCLUDE_DIR}
)

file(GLOB Sources code/*.cpp)

add_definitions(-fPIC)
add_library(scriptengine STATIC ${Sources})
target_link_libraries(scriptengine
    ${V8_LIBRARIES}
)
