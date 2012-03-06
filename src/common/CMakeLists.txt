include_directories(
    ${CEF_INCLUDE_DIRS}
)

file (GLOB Sources gcJSBase.cpp)

add_library(gcJSBase STATIC ${Sources})
