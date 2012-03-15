include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${Boost_INCLUDE_DIR}
)

file(GLOB Sources code/*.cpp)

add_library(threads STATIC ${Sources})

target_link_libraries(threads
    ${Boost_LIBRARIES}
)
