add_compiler_flags(-fPIC -pipe -fvisibility=hidden -Wl,-Bsymbolic-functions -lpthread -finline-functions -gcc)
add_compiler_flags(DEBUG -rdynamic -fno-omit-frame-pointer -g3)
add_compiler_flags(RELEASE -O2)
add_compiler_flags(CXX -fpermissive -std=c++0x)

#ignore some warnings
add_compiler_flags(-wd2928,10120)
set(ICC TRUE)
