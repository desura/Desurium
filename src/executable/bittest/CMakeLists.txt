include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/code
)

file(GLOB Sources code/*.c )

add_executable(desura_bittest ${Sources})

set(flags "")

if(CMAKE_COMPILER_IS_GNUCXX)
  set(compileFlags "-m32")
  set(linkFlags "-m32")
endif()

set_target_properties(desura_bittest PROPERTIES COMPILE_FLAGS ${compileFlags})
set_target_properties(desura_bittest PROPERTIES LINK_FLAGS ${linkFlags})

install(TARGETS desura_bittest
        DESTINATION "${BIN_INSTALL_DIR}")
