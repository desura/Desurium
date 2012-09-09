cmake_minimum_required(VERSION 2.8)
project(wxWidget_2_9_test)

find_package(wxWidgets REQUIRED)

include_directories(
  ${wxWidgets_INCLUDE_DIRS}
)
file(GLOB Sources *.cpp)
# wxWidgets config
add_definitions(
  -D__WXGTK__
  -D_LARGE_FILES
  -D_FILE_OFFSET_BITS=64
)
add_executable(wxWidget_2_9_test ${Sources})
target_link_libraries(
  wxWidget_2_9_test
  ${wxWidgets_LIBRARIES}	
)
