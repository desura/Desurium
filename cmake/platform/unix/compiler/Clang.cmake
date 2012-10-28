message("-- using clang")
add_compiler_flags(-fPIC -pipe -fvisibility=hidden -finline-functions)
add_compiler_flags(DEBUG -fno-omit-frame-pointer -g3)
add_compiler_flags(RELEASE -O2)
add_compiler_flags(CXX -std=c++0x)

if(DEBUG)
  add_linker_flags(-rdynamic)
endif()

set(CLANG TRUE)
