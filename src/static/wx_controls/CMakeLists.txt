include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${GLOBAL_INCLUDE_DIRS}
  ${GLOBAL_INCLUDE_DIRS}/wx_controls
  ${COMMON_INCLUDE_DIRS}
  ${GTK2_INCLUDE_DIRS}
  ${TINYXML_INCLUDE_DIRS}
  ${wxWidgets_INCLUDE_DIRS}
)

file(GLOB Headers code/*.h)
file(GLOB Sources code/*.cpp)

add_definitions(-fPIC)
add_library(wx_controls STATIC ${Sources})
target_link_libraries(wx_controls
    tinyxml
    ${GTK2_LIBRARIES}
    ${wxWidgets_LIBRARIES}
)
