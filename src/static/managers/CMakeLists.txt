include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
  ${Boost_INCLUDE_DIR}
  ${SQLITE3X_INCLUDE_DIRS}
)

file(GLOB Sources code/*.cpp)
file(GLOB SourcesWindow code/WindowManager.cpp)
list(REMOVE_ITEM Sources ${SourcesWindow})

if(WIN32)
  add_definitions(/D_LIB
                  /DwxUSE_UNICODE
				  /DWXUSINGDLL
				  /D_CRT_SECURE_NO_WARNINGS
				  /DUSE_WX_WIDGETS)
endif()

add_library(managers STATIC ${Sources} ${COMMON_SOURCE})
target_link_libraries(managers
    ${SQLITE3X_LIBRARIES}
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


