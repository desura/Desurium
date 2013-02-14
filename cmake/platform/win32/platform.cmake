################################################################################
# set some windows specific variables
################################################################################

set(SCRIPT_PREFIX bat)

# some boost options
add_definitions(-DBOOST_FILESYSTEM_VERSION=3 -DBOOST_ALL_NO_LIB=1)

# only define this if MFC was found
# VC express and mingw doesn't provide MFC, but other versions of VC do
if(MFC_FOUND)
  add_definitions(-DMFC_FOUND)
endif()

# some wxWidget options
macro(use_unicode_here)
  add_definitions(-D_UNICODE)
  add_definitions(-DUNICODE)
  add_definitions(-DwxUSE_UNICODE)
endmacro()

if(DEBUG)
    add_definitions(-DDEBUG -D_DEBUG)
else()
    add_definitions(-DNDEBUG -D_NDEBUG)
endif()

# ignore some warnings
add_linker_flags(/ignore:4006
                 /ignore:4099
				 /ignore:4221)
add_compiler_flags(/wd4996 /MP)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(64BIT TRUE)
  message("-- detected 64bit")
else()
  set(64BIT FALSE)
  message("-- detected 32bit")
endif()
