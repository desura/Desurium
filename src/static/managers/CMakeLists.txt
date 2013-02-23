include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${Boost_INCLUDE_DIR}
  ${SQLITE3X_INCLUDE_DIRS}
)

file(GLOB Sources code/*.cpp)
file(GLOB SourcesWindow code/WindowManager.cpp)
list(REMOVE_ITEM Sources ${SourcesWindow})

if(WIN32)
  use_unicode_here()
  add_definitions(-DWXUSINGDLL
				  -D_CRT_SECURE_NO_WARNINGS)
endif()

add_library(managers STATIC ${Sources} ${COMMON_SOURCE})
target_link_libraries(managers
  ${SQLITE3X_LIBRARIES}
  ${CMAKE_THREAD_LIBS_INIT}
)

include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${SQLITE3X_INCLUDE_DIRS}
  ${wxWidgets_INCLUDE_DIRS}
)

add_definitions(-DUSING_WX_WIDGETS)

add_library(managers_window STATIC ${SourcesWindow})
target_link_libraries(managers_window
    ${wxWidgets_LIBRARIES}
)


