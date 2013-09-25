################################################################################
# set some windows specific variables
################################################################################

# some boost options
add_definitions(-DBOOST_FILESYSTEM_VERSION=3 -DBOOST_ALL_NO_LIB=1)

# some wxWidget options
macro(use_unicode_here)
  add_definitions(-D_UNICODE -DUNICODE -DwxUSE_UNICODE)
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

set(WIN_TARGET 0x0502) # Windows XP SP2
set(WIN_SDK_MIN 0x0600) # Windows Vista
set(WIN_IE_VERSION 0x0603) # IE 6 SP2
add_definitions(-DWINVER=${WIN_TARGET}
                -D_WIN32_WINNT=${WIN_SDK_MIN}
                -D_WIN32_IE=${WIN_IE_VERSION})

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(64BIT TRUE)
  message("-- detected 64bit")
else()
  set(64BIT FALSE)
  message("-- detected 32bit")
endif()

macro(SetSharedRuntime target)
  if(MFC_FOUND)
    set(CMAKE_MFC_FLAG 2)
    set(MFC_DEF -D_AFXDLL)
  endif()

  if(DEBUG)
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS "/MDd ${MFC_DEF}")
  else()
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS "/MD ${MFC_DEF}")
  endif()
endmacro()

macro(SetStaticRuntime target)
  if(MFC_FOUND)
    set(CMAKE_MFC_FLAG 1)
  endif()

  if(DEBUG)
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS "/MTd")
  else()
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS "/MT")
  endif()
endmacro()

macro(SetUseMFC)
  set(CMAKE_MFC_FLAG 2)
  add_definitions(-DMFC_FOUND -DDONT_INCLUDE_WINH)
  # Visual Studio 9 needs this
  add_definitions(-D_BIND_TO_CURRENT_CRT_VERSION -D_BIND_TO_CURRENT_MFC_VERSION -DNO_WARN_MBCS_MFC_DEPRECATION)
endmacro()
