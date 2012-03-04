include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(COURGETTE_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/include/ PARENT_SCOPE)

file(GLOB CourgetteHeaders ./include/*.hpp)
file(GLOB CourgetteSources ./code/*.cpp ./include/*.hpp)

#add_library(courgette SHARED ${CourgetteSources})
