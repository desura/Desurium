include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../util_thread
  ${Boost_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR_S}   
)

file(GLOB Sources ${CMAKE_CURRENT_SOURCE_DIR}/../util_thread/code/*.cpp)

add_library(threads_s STATIC ${Sources})

target_link_libraries(threads_s
  ${Boost_SYSTEM_LIBRARY_S}
  ${Boost_THREAD_LIBRARY_S}
  ${Boost_CHRONO_LIBRARY_S}
)

SET_PROPERTY(TARGET threads_s                PROPERTY FOLDER "Static")
SetStaticRuntime(threads_s)