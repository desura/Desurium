include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB Sources code/*.cpp code/win32/*.cpp code/third_party/*.cpp)

add_library(util_bootloader STATIC ${Sources})

target_link_libraries(util_bootloader
    Psapi
    wintrust
)

if(WIN32)
  SetSharedRuntime(util_bootloader)
endif()

SET_PROPERTY(TARGET util_bootloader                PROPERTY FOLDER "Static")

if (WIN32)
	add_library(util_bootloader_s STATIC ${Sources})

	target_link_libraries(util_bootloader_s
		Psapi
		wintrust
	)

	SET_PROPERTY(TARGET util_bootloader_s                PROPERTY FOLDER "Static")
	SetStaticRuntime(util_bootloader_s)
endif()

set(UTIL_BOOTLOADER_INCLUDE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/code/win32" PARENT_SCOPE)
