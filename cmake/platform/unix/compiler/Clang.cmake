message("-- using clang")
add_compiler_flags(-fPIC -pipe -fvisibility=hidden -finline-functions)
add_compiler_flags(DEBUG -fno-omit-frame-pointer -g3)
add_compiler_flags(RELEASE -O2)

if(NOT APPLE)
  add_compiler_flags(CXX -std=c++0x)
  add_linker_flags(-Wl,--as-needed)
else()
  add_compiler_flags(-stdlib=libc++)
  add_compiler_flags(CXX -std=c++11)
  add_linker_flags(-stdlib=libc++)
endif()

if(DEBUG)
  add_linker_flags(-rdynamic)
endif()

set(CLANG TRUE)
