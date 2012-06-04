include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB Sources code/*.cpp code/third_party/*.cpp)

add_linker_flags(/NOD:MSVCRTD)

#add_definitions(-D_AFXDLL)
add_library(util_bootloader STATIC ${Sources})

target_link_libraries(util_bootloader
    Psapi
    wintrust
)
