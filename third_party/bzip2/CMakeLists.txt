include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/code
)

set(BZIP2_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include/ PARENT_SCOPE)

file(GLOB Headers ${BZIP2_INCLUDE_DIR}/bzlib.h)
file(GLOB Sources
  ./code/blocksort.c
  ./code/bzip2.c
  ./code/bzip2recover.c
  ./code/bzlib.c
  ./code/bzlib_private.h
  ./code/compress.c
  ./code/crctable.c
  ./code/decompress.c
  ./code/huffman.c
  ./code/randtable.c
)

add_library(bzip2 STATIC ${Sources} ${Headers})
  target_link_libraries(bzip2
)

set(BZIP2_LIBRARIES bzip2 PARENT_SCOPE)
SET_PROPERTY(TARGET bzip2                PROPERTY FOLDER "ThirdParty")

if (WIN32)
	add_library(bzip2_s STATIC ${Sources} ${Headers})
	  target_link_libraries(bzip2_s)

	set(BZIP2_LIBRARIES_S bzip2_s PARENT_SCOPE)
	SET_PROPERTY(TARGET bzip2_s                PROPERTY FOLDER "ThirdParty")

	SetStaticRuntime(bzip2_s)
endif()