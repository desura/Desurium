include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${Boost_INCLUDE_DIR}
)

file(GLOB Sources code/UtilFs.cpp)

if(WIN32)
  file(GLOB PlattFormSources code/UtilFs_win.cpp)
  add_linker_flags(/NOD:MSVCRTD)
else()
  file(GLOB PlattFormSources code/UtilFs_nix.cpp)
endif()

add_library(util_fs STATIC ${Sources} ${PlattFormSources})

target_link_libraries(util_fs
    ${Boost_LIBRARIES}
)
