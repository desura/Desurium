include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
)

file(GLOB Sources code/*.cpp code/third_party/*.cpp)

add_library(util_bootloader STATIC ${Sources})

target_link_libraries(util_bootloader
    Psapi
    wintrust
)

SET_PROPERTY(TARGET util_bootloader                PROPERTY FOLDER "Static")

if (WIN32)
	SetUseMFC()
	add_library(util_bootloader_s STATIC ${Sources})

	target_link_libraries(util_bootloader_s
		Psapi
		wintrust
	)

	SET_PROPERTY(TARGET util_bootloader_s                PROPERTY FOLDER "Static")
	SetStaticRuntime(util_bootloader_s)
endif()