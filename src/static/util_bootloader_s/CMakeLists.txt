include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../util_bootloader/code
)

if(WIN32)
	set(Win32Glob ../util_bootloader/code/win32/*.cpp)
	set(Win32Include ${CMAKE_CURRENT_SOURCE_DIR}/../util_bootloader/code/win32)
endif()

file(GLOB Sources ../util_bootloader/code/*.cpp ../util_bootloader/code/third_party/*.cpp ${Win32Glob})

add_library(util_bootloader_s STATIC ${Sources})

target_link_libraries(util_bootloader_s
	Psapi
	wintrust
)

SET_PROPERTY(TARGET util_bootloader_s                PROPERTY FOLDER "Static")
SetStaticRuntime(util_bootloader_s)

set(UTIL_BOOTLOADER_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/../util_bootloader/code ${Win32Include} PARENT_SCOPE)