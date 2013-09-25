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

if(WIN32)
  SetSharedRuntime(threads)
endif()

SET_PROPERTY(TARGET threads                PROPERTY FOLDER "Static")

if (WIN32)
	add_library(threads_s STATIC ${Sources})

	target_link_libraries(threads_s
	  ${Boost_SYSTEM_LIBRARY_S}
	  ${Boost_THREAD_LIBRARY_S}
	  ${Boost_CHRONO_LIBRARY_S}
	)

	SET_PROPERTY(TARGET threads_s                PROPERTY FOLDER "Static")
	SetStaticRuntime(threads_s)
endif()