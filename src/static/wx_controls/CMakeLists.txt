include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GLOBAL_INCLUDE_DIRS}/wx_controls
  ${Boost_INCLUDE_DIR}
  ${GTK2_INCLUDE_DIRS}
  ${wxWidgets_INCLUDE_DIRS}
)

file(GLOB Sources code/*.cpp)

if(WIN32)
  use_unicode_here()
  add_linker_flags(/NOD:MSVCRTD)
  add_definitions(-DWXUSINGDLL)
endif()

add_library(wx_controls STATIC ${Sources})

target_link_libraries(wx_controls
    ${GTK2_LIBRARIES}
    ${wxWidgets_LIBRARIES}
)

SET_PROPERTY(TARGET wx_controls                PROPERTY FOLDER "Static")