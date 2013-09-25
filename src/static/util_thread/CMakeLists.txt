include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${Boost_INCLUDE_DIR}
  ${Boost_INCLUDE_DIR_S}   
)

file(GLOB Sources code/*.cpp)

add_library(threads STATIC ${Sources})

if(WIN32)
  set(PLATFORM_LIBRARIES ${Boost_CHRONO_LIBRARY})
endif()

target_link_libraries(threads
  ${Boost_SYSTEM_LIBRARY}
  ${Boost_THREAD_LIBRARY}
  ${PLATFORM_LIBRARIES}
)

SET_PROPERTY(TARGET threads                PROPERTY FOLDER "Static")