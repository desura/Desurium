include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../util_bootloader
)

file(GLOB Sources ../util_bootloader/code/*.cpp ../util_bootloader/code/third_party/*.cpp)

SetUseMFC()
add_library(util_bootloader_s STATIC ${Sources})

target_link_libraries(util_bootloader_s
	Psapi
	wintrust
)

SET_PROPERTY(TARGET util_bootloader_s                PROPERTY FOLDER "Static")
SetStaticRuntime(util_bootloader_s)
