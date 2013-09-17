include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CEF_INCLUDE_DIRS}
  ${V8_INCLUDE_DIR}
)

file(GLOB Sources code/*.cpp)

if(WIN32)
  set(WINDOWS_LIBRARIES Winmm)
  add_linker_flags(/NOD:MSVCRTD)
endif()

add_library(scriptengine STATIC ${Sources})
target_link_libraries(scriptengine
    gcJSBase
    threads
    util
    util_fs
    ${V8_LIBRARIES}
)

SET_PROPERTY(TARGET scriptengine                PROPERTY FOLDER "Static")