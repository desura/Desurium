include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(CEF_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} PARENT_SCOPE)

file(GLOB CefHeaders ./include/*.hpp)
file(GLOB CefSources ./code/*.cpp ./include/*.hpp)

#add_library(cef SHARED ${CourgetteSources})
