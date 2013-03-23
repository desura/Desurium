message("-- using GCC")
add_compiler_flags(-fPIC -pipe -fvisibility=hidden -finline-functions)
add_compiler_flags(DEBUG -fno-omit-frame-pointer -g3)
add_compiler_flags(RELEASE -O2)
add_compiler_flags(CXX -fpermissive -std=c++0x)
add_linker_flags(-Bsymbolic-functions)

if(NOT APPLE)
  add_linker_flags(-Wl,--as-needed)
endif()


if(DEBUG)
  add_linker_flags(-rdynamic)
endif()

set(GCC TRUE)
